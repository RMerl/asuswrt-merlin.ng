How to review a patch
=====================

Some folks have said that they'd like to review patches more often, but they
don't know how.

So, here are a bunch of things to check for when reviewing a patch!

Note that if you can't do every one of these, that doesn't mean you can't do
a good review!  Just make it clear what you checked for and what you didn't.


Top-level smell-checks
----------------------

(Difficulty: easy)

- Does it compile with `--enable-fatal-warnings`?

- Does `make check-spaces` pass?

- Does `make check-changes` pass?

- Does it have a reasonable amount of tests?  Do they pass?  Do they leak
  memory?

- Do all the new functions, global variables, types, and structure members have
 documentation?

- Do all the functions, global variables, types, and structure members with
  modified behavior have modified documentation?

- Do all the new torrc options have documentation?

- If this changes Tor's behavior on the wire, is there a design proposal?

- If this changes anything in the code, is there a "changes" file?


Let's look at the code!
-----------------------

- Does the code conform to CodingStandards.txt?

- Does the code leak memory?

- If two or more pointers ever point to the same object, is it clear which
  pointer "owns" the object?

- Are all allocated resources freed?

- Are all pointers that should be const, const?

- Are `#defines` used for 'magic' numbers?

- Can you understand what the code is trying to do?

- Can you convince yourself that the code really does that?

- Is there duplicated code that could be turned into a function?


Let's look at the documentation!
--------------------------------

- Does the documentation confirm to CodingStandards.txt?

- Does it make sense?

- Can you predict what the function will do from its documentation?


Let's think about security!
---------------------------

- If there are any arrays, buffers, are you 100% sure that they cannot
  overflow?

- If there is any integer math, can it overflow or underflow?

- If there are any allocations, are you sure there are corresponding
  deallocations?

- Is there a safer pattern that could be used in any case?

- Have they used one of the Forbidden Functions?

(Also see your favorite secure C programming guides.)
