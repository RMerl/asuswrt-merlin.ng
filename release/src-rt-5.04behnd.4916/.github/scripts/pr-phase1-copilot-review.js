#!/usr/bin/env node
// Phase 1 Copilot PR review — v1 DRAFT, shadow-mode pilot.
// Guideline source of truth: https://github.com/ASUS-WL/pr-review-guideline (GUIDELINE.md §2/§3).
// Shared verbatim across the pilot repos (axhnd + behnd) — expect to revise once real PR
// traffic is observed on either. It intentionally never fails the job (shadow mode): see the
// workflow file's header for why.
//
// KNOWN GAPS (do not treat this as finished):
//   - File-scope filtering below is hardcoded per repo shortname, mirroring
//     pr-review-guideline/rules/t-score-rules.yml's `file_scope` block by hand. If that YAML
//     changes, this must be updated manually — no automated sync yet.
//   - Copilot CLI output parsing is best-effort text matching, not a strict schema. If Copilot's
//     reply doesn't follow the requested format, this falls back to dumping the raw reply.
//   - "significant change since last review" is tracked via a hidden marker storing the last
//     reviewed head SHA in the bot's own PR comment. If the comment is deleted, the next
//     synchronize is treated as a first review (fail open, not closed).
//   - Phase1-A (GUIDELINE.md §3.1) judges commit-message clarity from the message text alone
//     (plus whatever diff context is already in the same prompt) — it does NOT search git
//     history to detect an actual apply→revert→reapply sequence across separate commits/PRs.
//     Criterion 3 of §3.1 ("if this looks like a revert/reapply, is the reason explained") is
//     judged only from what's visible in *this* PR's own commits, not a historical search.

const { execSync } = require("child_process");
const fs = require("fs");
const path = require("path");

const COMMENT_MARKER = "<!-- pr-phase1-copilot-review:guideline-g2 -->";
const SIGNIFICANT_CHANGE_FILES = 3;
const SIGNIFICANT_CHANGE_LINES = 50;

const REPO_FILE_SCOPE = {
  "asuswrt-src-rt-5.04behnd.4916": null, // null = no filter, everything in scope
  "asuswrt-src-rt-5.04axhnd.675x": null,
  "asuswrt-src": ["release/src/router/rc/", "release/src/router/shared/"],
};

function sh(cmd, opts = {}) {
  return execSync(cmd, { encoding: "utf8", maxBuffer: 1024 * 1024 * 64, ...opts }).trim();
}

// POSIX single-quote escaping for paths interpolated into shell commands (e.g. a changed file's
// name from the PR's own diff, which a PR author controls) — avoids malformed/unsafe commands if
// a path ever contains a single quote.
function shQuote(str) {
  return `'${str.replace(/'/g, "'\\''")}'`;
}

function getEnv(name, required = true) {
  const v = process.env[name];
  if (required && !v) throw new Error(`Missing required env var ${name}`);
  return v;
}

function isInScope(filePath, scopePrefixes) {
  if (scopePrefixes === null) return true;
  return scopePrefixes.some((p) => filePath.startsWith(p));
}

function getChangedFiles(baseSha, headSha) {
  // Three-dot diff (against the merge-base), not two-dot: base_sha is the *current* tip of the
  // target branch, which keeps moving as unrelated PRs merge into it. A two-dot `git diff base
  // head` computes the raw tree difference between those two snapshots and doesn't account for
  // that drift — any commit that landed on the target branch after this PR's branch forked shows
  // up as if this PR were reverting it, even though the PR never touched those files. Three-dot
  // diffs against the merge-base instead, matching what GitHub's own PR page diff shows.
  const out = sh(`git diff --numstat ${baseSha}...${headSha}`);
  if (!out) return [];
  return out
    .split("\n")
    .filter(Boolean)
    .map((line) => {
      const [added, removed, file] = line.split("\t");
      return {
        file,
        added: added === "-" ? 0 : parseInt(added, 10),
        removed: removed === "-" ? 0 : parseInt(removed, 10),
      };
    });
}

function getCommitMessages(baseSha, headSha) {
  const out = sh(`git log --format="%H%x01%B%x02" ${baseSha}..${headSha}`);
  return out
    .split("\x02")
    .filter((s) => s.trim())
    .map((s) => {
      const [sha, msg] = s.split("\x01");
      return { sha: sha.trim(), message: msg.trim() };
    });
}

// --- M1: T-score formula recompute (ported from the legacy pr-t-score-check.js, unchanged formula) ---
const WEIGHTS = { D: 0.3, RE: 0.1, CN: 0.3, ST: 0.2, PF: 0.1 };
const SPECIAL_CASE_KEYS = ["D", "CN", "ST", "PF"];
const T_LINE_RE = /T:\s*([\d.]+)\s*\[\s*D(\d+)\s*,\s*RE(\d+)\s*,\s*CN(\d+)\s*,\s*ST(\d+)\s*,\s*PF(\d+)\s*\]/i;
const PRIORITY_LINE_RE = /^Priority:.*$/im;
const OLD_PRIORITY_RE = /Priority:\s*\d+(\.\d+)?\s*$/im;

function expectedT(scores) {
  const special = SPECIAL_CASE_KEYS.some((k) => scores[k] === 5);
  if (special) return 10.0;
  const raw =
    WEIGHTS.D * scores.D + WEIGHTS.RE * scores.RE + WEIGHTS.CN * scores.CN + WEIGHTS.ST * scores.ST + WEIGHTS.PF * scores.PF;
  return 1 + 2.25 * (raw - 1);
}

function checkM1(message) {
  const priorityLine = (message.match(PRIORITY_LINE_RE) || [null])[0];
  if (!priorityLine) return { status: "no-priority-line" };
  const m = message.match(T_LINE_RE);
  if (!m) {
    if (OLD_PRIORITY_RE.test(priorityLine)) return { status: "old-format-exempt" };
    return { status: "unparsed-format" };
  }
  const [, claimedTStr, d, re, cn, st, pf] = m;
  const claimedT = parseFloat(claimedTStr);
  const scores = { D: Number(d), RE: Number(re), CN: Number(cn), ST: Number(st), PF: Number(pf) };
  const expected = expectedT(scores);
  const diff = Math.abs(claimedT - expected);
  return { status: "parsed", claimedT, expected, scores, mismatch: diff > 0.05 };
}

// M1's per-commit line text plus one aggregate symbol for the "== 卡關審查結果 ==" summary.
// Priority (highest first): any commit with a genuine, positively-detected mismatch → 失敗❌;
// else any commit actually parsed cleanly → 成功✅; else (every commit was no-priority-line /
// old-format-exempt / unparsed-format — nothing was actually checkable) → 未檢查➖. Same
// fault-tolerance principle as verdictSymbol(): only a positively-confirmed problem counts as a
// failure, "couldn't check" is neutral, not a pass or a fail.
function renderM1(m1Results) {
  const lines = m1Results.map((r) => {
    if (r.status === "parsed") {
      return `- commit ${r.sha}: ${r.mismatch ? "🚫 T分數計算不一致" : "✅ T分數重算一致"}（宣稱 ${r.claimedT}，重算 ${r.expected.toFixed(2)}）`;
    }
    return `- commit ${r.sha}: ${r.status}`;
  });
  const anyMismatch = m1Results.some((r) => r.status === "parsed" && r.mismatch);
  const anyClean = m1Results.some((r) => r.status === "parsed" && !r.mismatch);
  const symbol = anyMismatch ? "失敗❌" : anyClean ? "成功✅" : "未檢查➖";
  return { text: lines.join("\n") || "（沒有可解析的 Priority 行）", symbol };
}

// --- Marker / prior-comment lookup (for idempotent updates + "since last review" diffing) ---
function findExistingComment(repoFull, prNumber) {
  const json = sh(`gh api repos/${repoFull}/issues/${prNumber}/comments --paginate`);
  const comments = JSON.parse(json);
  return comments.find((c) => c.body && c.body.includes(COMMENT_MARKER));
}

function getLastReviewedSha(existingComment) {
  if (!existingComment) return null;
  const m = existingComment.body.match(/<!-- last-reviewed-sha:([0-9a-f]{40}) -->/);
  return m ? m[1] : null;
}

// Plain (non-HTML-comment) tokens on purpose: the whole marker+content block gets wrapped in
// ONE HTML comment in main() to actually hide it (see that comment for why the previous
// self-closing-comment markers didn't work — HTML/Markdown comments don't nest, so a marker
// that was itself a complete `<!-- ... -->` closed the wrapping comment immediately). These are
// matched with plain indexOf/slice, not HTML-parsed, so their own text has no comment syntax.
const AI_SECTION_START = "[[last-ai-section:start]]";
const AI_SECTION_END = "[[last-ai-section:end]]";

// The comment body is fully regenerated every run (see upsertComment), so a run that doesn't
// re-trigger Copilot (reopened, or "not significant" on synchronize) would otherwise silently
// drop the last real AI assessment. Persist it across runs via a marker pair instead.
//
// Stored as JSON (the parsed verdict object + evaluatedAt/headShaShort), not pre-rendered
// markdown text — the "== 卡關審查結果 ==" summary line needs a real PASS/FAIL/NOT_TRIGGERED
// verdict to pick a symbol even when this run is just carrying forward an old result (2026-09-11
// format change). Scraping symbols back out of old rendered text would be fragile; re-rendering
// from the same structured data used for a fresh run is not.
// KNOWN LIMITATION (not handled in this v1 draft): if Copilot's raw reply ever contains a literal
// `-->`, this naive indexOf-based extraction would truncate early, and separately the wrapping
// HTML comment in main() would also close early, leaking the remainder into the visible render.
// Low probability, not guarded.
function getLastAiSnapshot(existingComment) {
  if (!existingComment) return null;
  const start = existingComment.body.indexOf(AI_SECTION_START);
  const end = existingComment.body.indexOf(AI_SECTION_END);
  if (start === -1 || end === -1 || end < start) return null;
  const raw = existingComment.body.slice(start + AI_SECTION_START.length, end).trim();
  if (!raw) return null;
  try {
    return JSON.parse(raw);
  } catch {
    // Pre-2026-09-11 comments stored pre-rendered markdown here, not JSON — treat as "no
    // history" rather than crash; the next successful AI run replaces it with the new format.
    return null;
  }
}

// Verdict → the three-way symbol used in both the "== 卡關審查結果 ==" summary line and next to
// each detail item's 判定. NOT_TRIGGERED is a real, expected outcome (§3.3: no logic change to
// judge) — showing ❌ would misreport it as a failure; showing ✅ would misreport it as "checked,
// fine". A parse failure (Copilot's reply didn't match the expected fields) is neither a verdict
// nor "nothing to check" — it means we don't actually know, so it gets its own ⚠️ symbol distinct
// from NOT_TRIGGERED, same fault-tolerance spirit as Phase1-A's default-to-PASS-but-flagged.
function verdictSymbol(verdict, parseFailed) {
  if (parseFailed) return "解析異常⚠️";
  if (verdict === "PASS") return "成功✅";
  if (verdict === "FAIL") return "失敗❌";
  if (verdict === "NOT_TRIGGERED") return "未觸發➖";
  return "未知⚠️"; // defensive: Copilot returned a verdict token outside {PASS,FAIL,NOT_TRIGGERED}
}

function upsertComment(repoFull, prNumber, body, existingComment) {
  const bodyFile = path.join("/tmp", "pr-phase1-comment-body.md");
  fs.writeFileSync(bodyFile, body);
  if (existingComment) {
    sh(`gh api -X PATCH repos/${repoFull}/issues/comments/${existingComment.id} -F body=@${bodyFile}`);
  } else {
    sh(`gh api repos/${repoFull}/issues/${prNumber}/comments -F body=@${bodyFile}`);
  }
}

// --- Copilot CLI invocation ---
function buildPrompt({ guidelineExcerpt, commitMessages, diffText }) {
  return `你是這個 repo 的 PR 覆核助手，依照下面附上的 guideline 判斷這次 PR。

# Guideline（節錄，來自 pr-review-guideline repo 的 GUIDELINE.md，§3/§5/§6）
${guidelineExcerpt}

# 這次 PR 的 commit message
${commitMessages}

# 這次 PR 的 code diff（已依檔案範圍過濾）
${diffText}

請針對以下三項做判斷，用固定格式回覆，不要有其他文字：

## Phase1-A
（commit message 正確明確——判斷 commit message 敘述文字，不含 Priority 那行，是否清楚交代：
1. 改了什麼、為什麼改——不能只有 CS 案號或「fix bug」這類沒有實質內容的敘述；
2. 若是 bug fix，觸發條件/症狀有沒有描述清楚，跟宣稱的 D/RE/CN/ST/PF 子分數是否對得起來；
3. 若這次改動看起來像是同一路徑/功能的 revert 或 reapply，有沒有交代原因（為什麼上次要revert、
   這次為什麼又要重新apply）。
容錯原則：判斷不出明確結論時預設 PASS；只有明顯不清楚/不正確（例如訊息只有案號、完全沒有描述性
文字）才判 FAIL，避免誤傷率過高擋住正常PR。）
判定: <PASS 或 FAIL>
理由: <一句話>

## Phase1-B
（PR評分T部分正確）
判定: <PASS 或 FAIL>
重新估算: [D<x>,RE<x>,CN<x>,ST<x>,PF<x>]
理由: <一句話>

## Phase1-C
（實作正確性——判斷這次diff的實際程式碼實作，跟commit message描述的內容、以及正確的函式引用/邏輯
行為是否互相吻合。判斷依據是你對這次diff本身的code review，不是拿規則清單去比對命中哪一條。上面
guideline §6列的R1~R13是歷史已知風險模式的參考案例，可以用來對照「這次的問題長得像哪個已知模式」，
但不是判定的唯一依據——即使沒有命中任何已知案例，只要code review判斷實作有問題或跟描述不吻合，
一樣判FAIL；反過來即使路徑/特徵命中了某個已知案例，只要這次實際寫法沒有真正的問題，就不要判FAIL。
只有當這次diff完全沒有需要判斷實作正確性的程式邏輯變動時（例如純文件、純格式調整），才回
NOT_TRIGGERED。）
判定: <PASS 或 FAIL 或 NOT_TRIGGERED>
參考案例: <最相似的R1~R13之一，或無——僅供對照標籤，不影響判定>
理由: <一句話>
`;
}

function callCopilot(prompt) {
  // NOTE: diffs/long text must be embedded directly in -p — `--attachment` only accepts
  // images/native documents, not plain text (verified against Copilot CLI v1.0.65).
  const escaped = prompt.replace(/'/g, "'\\''");
  let result;
  try {
    const text = sh(`copilot -p '${escaped}' --allow-all-tools -s`);
    result = { ok: true, text };
  } catch (err) {
    // IMPORTANT: err.message (from execSync) includes the *entire failed command line*, i.e. the
    // whole prompt we just sent — which itself contains our own "判定: <PASS 或 FAIL>"-style format
    // instructions. A failure must NEVER be run through parseCopilotReply() as if it were a real
    // reply, or those literal template placeholders get regex-matched out of our own prompt and
    // misreported as if Copilot had actually answered "PASS". (This is exactly what happened on
    // the first pilot run — see GUIDELINE.md/plan-doc history, 2026-08-31.) Only keep a short,
    // clearly-separate error summary here — never feed err.message into the parser.
    const fullText = String(err.stderr || err.message || err);
    // execSync's err.message leads with "Command failed: <the whole command, including our giant
    // escaped prompt>" — that line is useless as a "summary". Prefer the first line that actually
    // looks like an error (contains "Error"/"error"); only fall back to the command-failed line if
    // nothing better is found.
    const lines = fullText.split("\n").map((l) => l.trim()).filter(Boolean);
    const errorSummary = lines.find((l) => /error/i.test(l)) || lines[0] || fullText;
    result = { ok: false, errorSummary, errorFull: fullText };
  }
  // Always log to the Action's own run log — the PR comment gets fully regenerated every run, so
  // this is the only durable record once a later run overwrites it.
  console.log("[phase1] ===== Copilot call result start =====");
  console.log(JSON.stringify(result, null, 2));
  console.log("[phase1] ===== Copilot call result end =====");
  return result;
}

function parseCopilotReply(reply) {
  // Colon and glyph tolerance are insurance, not the fix: GUIDELINE.md §8 (2026-09-10) is the
  // single source of truth for the field format Copilot is asked to use (half-width colon,
  // traditional 參 U+53C3). Copilot still doesn't guarantee it follows that every time, and
  // trading a whole Phase1-B/C verdict for a single wrong character is a bad trade — so this
  // also matches full-width colons (：) and the simplified glyph (参) as a fallback. See
  // CHANGELOG.md 2026-09-10 for the incident history (asuswrt-src #18765/#18811/#18933, behnd #323).
  const a = /## Phase1-A[\s\S]*?判定\s*[:：]\s*(\S+)[\s\S]*?理由\s*[:：]\s*(.*)/i.exec(reply);
  const b = /## Phase1-B[\s\S]*?判定\s*[:：]\s*(\S+)[\s\S]*?重新估算\s*[:：]\s*(.*)[\s\S]*?理由\s*[:：]\s*(.*)/i.exec(reply);
  const c = /## Phase1-C[\s\S]*?判定\s*[:：]\s*(\S+)[\s\S]*?[参參]考案例\s*[:：]\s*(.*)[\s\S]*?理由\s*[:：]\s*(.*)/i.exec(reply);
  return {
    // Fault-tolerance principle (GUIDELINE.md §3.1): default to PASS when Copilot's reply can't be
    // parsed for Phase1-A specifically — unclear judgment should not block a normal PR. Phase1-B/C
    // have no such default; an unparseable reply there just shows as unparsed, no verdict asserted.
    // `parseFailed` lets the rendered comment surface this visibly instead of letting an unparsed
    // Phase1-A look identical to a real PASS (that silent-failure gap is what let two genuine FAILs
    // get dropped without any visible sign — see CHANGELOG.md 2026-09-10).
    phase1A: a
      ? { verdict: a[1], reason: a[2].trim(), parseFailed: false }
      : { verdict: "PASS", reason: "無法解析 Copilot 回覆，依容錯原則預設通過", parseFailed: true },
    phase1B: b
      ? { verdict: b[1], rescored: b[2].trim(), reason: b[3].trim(), parseFailed: false }
      : { parseFailed: true },
    // `refCase` (2026-09-03起「參考案例」，原名「命中規則」) is informational only — Phase1-C's
    // verdict is a direct code-review judgment, not gated by whether this matches R1/R2/R5/R6.
    phase1C: c
      ? { verdict: c[1], refCase: c[2].trim(), reason: c[3].trim(), parseFailed: false }
      : { parseFailed: true },
    raw: reply,
  };
}

// Renders the "== 詳細說明 ==" numbered items 1-3 (卡關一/二/三) plus each phase's summary
// symbol, from a snapshot shaped like { evaluatedAt, headShaShort, phase1A, phase1B, phase1C,
// raw } — either freshly built this run from parseCopilotReply()'s output, or JSON.parse'd from
// a carried-forward marker (see getLastAiSnapshot). `snapshot` is null when Copilot has never
// successfully run on this PR at all (no fresh result, no history to carry forward either).
function renderAiDetail(snapshot) {
  if (!snapshot) {
    return {
      symbols: { a: "尚無評估⚠️", b: "尚無評估⚠️", c: "尚無評估⚠️" },
      timestampLine: "",
      warningLine: "",
      detailText: `1. 資訊正確性 (commit 資訊是否明確)
（尚無評估）

2. 評分正確性 (由Copilot透過diff進行評分)
（尚無評估）

3. 實作正確性 (透過code review判斷)
（尚無評估）`,
      rawReplyBlock: "",
    };
  }
  const { phase1A, phase1B, phase1C, evaluatedAt, headShaShort, raw } = snapshot;
  const anyParseFailed = phase1A.parseFailed || phase1B.parseFailed || phase1C.parseFailed;
  return {
    symbols: {
      a: verdictSymbol(phase1A.verdict, phase1A.parseFailed),
      b: verdictSymbol(phase1B.verdict, phase1B.parseFailed),
      c: verdictSymbol(phase1C.verdict, phase1C.parseFailed),
    },
    // Positioned inside item "4. M1 重新確認分數" in the final body template (2026-09-11), not
    // above item 1 — kept here as a distinct field anyway since it's about the AI's own
    // evaluation (keyed to headShaShort), not M1's.
    timestampLine: `_評估時間：${evaluatedAt}（commit ${headShaShort}）_`,
    // Surface parsing failures instead of letting them hide behind a default PASS or a bland
    // "unparseable" line — see CHANGELOG.md 2026-09-10 (two real FAILs were silently dropped
    // this way before this warning existed).
    warningLine: anyParseFailed
      ? "⚠️ **解析異常**：Copilot 這次回覆有欄位無法用預期格式解析（下方各項目已標記），判定結果不代表 Copilot 真正給出的結論——請展開「Copilot 原始回覆」確認實際內容。"
      : "",
    detailText: `1. 資訊正確性 (commit 資訊是否明確)${phase1A.parseFailed ? " ⚠️ 解析失敗，以下為容錯預設值" : ""}
判定: ${phase1A.verdict}
理由: ${phase1A.reason}

2. 評分正確性 (由Copilot透過diff進行評分)${phase1B.parseFailed ? " ⚠️ 解析失敗" : ""}
${phase1B.parseFailed ? "（無法解析 Copilot 回覆，原始內容附下）" : `判定: ${phase1B.verdict}\n重新估算: ${phase1B.rescored}\n理由: ${phase1B.reason}`}

3. 實作正確性 (透過code review判斷)${phase1C.parseFailed ? " ⚠️ 解析失敗" : ""}
${phase1C.parseFailed ? "（無法解析 Copilot 回覆）" : `判定: ${phase1C.verdict}\n參考案例: ${phase1C.refCase}\n理由: ${phase1C.reason}`}`,
    rawReplyBlock: `<details><summary>Copilot 原始回覆</summary>

\`\`\`
${raw}
\`\`\`

</details>`,
  };
}

function main() {
  const prNumber = getEnv("PR_NUMBER");
  const baseSha = getEnv("PR_BASE_SHA");
  const headSha = getEnv("PR_HEAD_SHA");
  const eventAction = getEnv("EVENT_ACTION");
  const repoShortname = getEnv("REPO_SHORTNAME");
  const guidelineDir = getEnv("GUIDELINE_DIR");
  const repoFull = `${process.env.GITHUB_REPOSITORY || "ASUS-WL/" + repoShortname}`;

  console.log(`[phase1] event=${eventAction} pr=${prNumber} base=${baseSha} head=${headSha}`);

  // 2026-09-02 confirmed (GUIDELINE.md §2): reopened skips EVERYTHING, including M1 — not just the
  // Copilot AI call. Reopen doesn't guarantee new code, and if this PR was already reviewed there's
  // nothing to gain from redoing even the cheap mechanical check. No comment is posted/updated here.
  if (eventAction === "reopened") {
    console.log("[phase1] reopened → skipping all checks (M1 + Copilot AI) per GUIDELINE.md §2, no comment posted.");
    return;
  }

  const commits = getCommitMessages(baseSha, headSha);
  const m1Results = commits.map((c) => ({ sha: c.sha.slice(0, 10), ...checkM1(c.message) }));
  const m1 = renderM1(m1Results);

  if (!(repoShortname in REPO_FILE_SCOPE)) {
    throw new Error(`No REPO_FILE_SCOPE entry for REPO_SHORTNAME="${repoShortname}" — check the workflow env var against the REPO_FILE_SCOPE map in this script.`);
  }
  const scopePrefixes = REPO_FILE_SCOPE[repoShortname];
  const allChanged = getChangedFiles(baseSha, headSha);
  const inScope = allChanged.filter((f) => isInScope(f.file, scopePrefixes));

  const existingComment = findExistingComment(repoFull, prNumber);
  const lastReviewedSha = getLastReviewedSha(existingComment);

  let shouldRunAi = true; // eventAction is "opened" or "synchronize" here — "reopened" returned above
  let sinceNote = "";
  if (eventAction === "synchronize") {
    if (lastReviewedSha) {
      const incremental = getChangedFiles(lastReviewedSha, headSha);
      const filesChanged = incremental.length;
      const linesChanged = incremental.reduce((s, f) => s + f.added + f.removed, 0);
      const significant = filesChanged >= SIGNIFICANT_CHANGE_FILES || linesChanged >= SIGNIFICANT_CHANGE_LINES;
      sinceNote = `自上次覆核（${lastReviewedSha.slice(0, 10)}）以來：${filesChanged} 檔／${linesChanged} 行`;
      shouldRunAi = significant;
    } else {
      sinceNote = "找不到上次覆核紀錄，視為第一次覆核";
    }
  }

  const lastSnapshot = getLastAiSnapshot(existingComment);
  let snapshot = lastSnapshot; // carried forward unless this run produces a fresh one
  let statusNote = ""; // short prose explaining *why* the detail below is/isn't fresh this run
  let errorBlock = "";
  if (shouldRunAi && inScope.length > 0) {
    const guidelinePath = path.join(guidelineDir, "GUIDELINE.md");
    const guidelineExcerpt = fs.existsSync(guidelinePath)
      ? fs.readFileSync(guidelinePath, "utf8").slice(0, 20000)
      : "[guideline repo 內容尚未 checkout 到，或 GUIDELINE.md 還未合併進 main — 這是 v1 draft 已知的排序依賴]";
    // Three-dot for the same reason as getChangedFiles() above.
    const diffText = sh(`git diff ${baseSha}...${headSha} -- ${inScope.map((f) => shQuote(f.file)).join(" ")}`).slice(0, 40000);
    const commitMsgText = commits.map((c) => `commit ${c.sha.slice(0, 10)}:\n${c.message}`).join("\n\n");
    const prompt = buildPrompt({ guidelineExcerpt, commitMessages: commitMsgText, diffText });
    const result = callCopilot(prompt);
    if (!result.ok) {
      // Never parse a failure as if it were a real reply (see callCopilot's comment for why).
      // Deliberately do NOT overwrite `snapshot` here — keep whatever last *successful*
      // assessment was already stored, so a transient/config failure doesn't erase real history.
      errorBlock = `🔴 **Copilot CLI 呼叫失敗**（本次未能評估，非卡關——shadow mode 不擋 PR）

錯誤摘要：\`${result.errorSummary}\`

<details><summary>完整錯誤內容</summary>

\`\`\`
${result.errorFull}
\`\`\`

</details>`;
      statusNote = lastSnapshot ? "_以下為上次成功覆核的結果，供參考：_" : "_目前沒有任何過去成功覆核的紀錄。_";
    } else {
      const parsed = parseCopilotReply(result.text);
      snapshot = {
        evaluatedAt: new Date().toISOString(),
        headShaShort: headSha.slice(0, 10),
        phase1A: parsed.phase1A,
        phase1B: parsed.phase1B,
        phase1C: parsed.phase1C,
        raw: parsed.raw,
      };
    }
  } else if (!shouldRunAi) {
    // Only reachable for synchronize events that failed the significant-change check —
    // "reopened" already returned at the top of main() and never reaches here.
    statusNote = lastSnapshot
      ? `_本次未重新觸發 Copilot AI 覆核（${sinceNote}）——以下為上次覆核結果：_`
      : `_本次未觸發 Copilot AI 覆核：${sinceNote}，且找不到過去的覆核紀錄。_`;
  } else {
    statusNote = lastSnapshot
      ? "_本次改動不在檔案範圍內，未觸發覆核——以下為上次覆核結果：_"
      : "_本次改動不在檔案範圍內，未觸發覆核，且找不到過去的覆核紀錄。_";
  }

  const detail = renderAiDetail(snapshot);
  // Timestamp deliberately excluded here — 2026-09-11 internal review moved it to sit inside
  // item "4. M1 重新確認分數" in the body template below, not above item 1.
  const detailBody = [statusNote, errorBlock, detail.warningLine, detail.detailText].filter(Boolean).join("\n\n");

  // AI_SECTION_START/END used to each be a complete, self-closing HTML comment
  // (`<!-- last-ai-section:start -->` ... content ... `<!-- last-ai-section:end -->`), with the
  // stored content rendered plainly in between. A self-closing HTML comment only hides itself,
  // not the surrounding text — so that content rendered fully visible, duplicating the whole
  // detail block on every comment (see 2026-09-10 asuswrt-src #19011 screenshot). The first
  // attempt at fixing this wrapped the marker pair + content in one outer `<!-- ... -->`, but
  // that's ALSO broken: HTML/Markdown comments don't nest, and each marker was itself a complete
  // `<!-- ... -->`, so its own `-->` closed the outer comment immediately, leaving everything
  // after it visible again — confirmed via the rendered comment HTML, not just by reasoning
  // about it. The actual fix is the plain (non-comment) marker tokens above, so nothing inside
  // this block can prematurely close the one real wrapping comment below.
  // Known limitation: if Copilot's raw reply ever contains a literal "-->", this wrapping
  // comment still closes early and the remainder leaks into the visible render. Low
  // probability, not guarded.
  const body = `${COMMENT_MARKER}
<!-- last-reviewed-sha:${headSha} -->
## [Guideline] 卡關機制與案例查核（SHADOW MODE — 目前只留言，不卡關）

== 卡關審查結果 ==
1. 資訊正確性 : ${detail.symbols.a}
2. 評分正確性 : ${detail.symbols.b}
3. 實作正確性 : ${detail.symbols.c}
4. M1 重新確認分數 : ${m1.symbol}

== 詳細說明 ==
${detailBody}

4. M1 重新確認分數
${m1.text}
${detail.timestampLine}

${detail.rawReplyBlock}

<!-- 以下為隱藏 marker，供下次 run 讀取「上次真正跑過 AI 的結果」，不會顯示在畫面上
${AI_SECTION_START}
${JSON.stringify(snapshot)}
${AI_SECTION_END}
-->

---
_這是 pilot 階段的 shadow-mode 留言，不影響 PR 合併。整合邏輯與規則內容見
[pr-review-guideline](https://github.com/ASUS-WL/pr-review-guideline) 的 GUIDELINE.md。_
`;

  upsertComment(repoFull, prNumber, body, existingComment);
  console.log("[phase1] comment posted/updated. Shadow mode: exiting 0 regardless of verdict.");
}

main();
