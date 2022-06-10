# Getting started in Tor development

Congratulations!  You've found this file, and you're reading it!  This
means that you might be interested in getting started in developing Tor.

(_This guide is just about Tor itself--the small network program at the
heart of the Tor network--and not about all the other programs in the
whole Tor ecosystem._)

If you are looking for a more bare-bones, less user-friendly information
dump of important information, you might like reading the
[doxygen output](https://src-ref.docs.torproject.org/tor/index.html).
You probably should skim some of the topic headings there before you write
your first patch.

## Required background

First, I'm going to assume that you can build Tor from source, and that
you know enough of the C language to read and write it.  (See the README
file that comes with the Tor source for more information on building it,
and any high-quality guide to C for information on programming.)

I'm also going to assume that you know a little bit about how to use
Git, or that you're able to follow one of the several excellent guides
at [git-scm](https://git-scm.org) to learn.

Most Tor developers develop using some Unix-based system, such as GNU/Linux,
BSD, or macOS.  It's okay to develop on Windows if you want, but you're
going to have a more difficult time.

## Getting your first patch into Tor

Once you've reached this point, here's what you need to know.

  1. Get the source.

     We keep our source under version control in Git.  To get the latest
     version, run:

     ```console
     $ git clone https://git.torproject.org/git/tor
     ```

     This will give you a checkout of the main branch.  If you're
     going to fix a bug that appears in a stable version, check out the
     appropriate "maint" branch, as in:

     ```console
     $ git checkout maint-0.4.3
     ```

  2. Find your way around the source.

     Our overall code structure is explained in our
     [source documentation](https://src-ref.docs.torproject.org/tor/index.html).

     Find a part of the code that looks interesting to you, and start
     looking around it to see how it fits together!

     We do some unusual things in our codebase.  Our testing-related
     practices and kludges are explained in `doc/HACKING/WritingTests.md`.

     If you see something that doesn't make sense, we love to get
     questions!

  3. Find something cool to hack on.

     You may already have a good idea of what you'd like to work on, or
     you might be looking for a way to contribute.

     Many people have gotten started by looking for an area where they
     personally felt Tor was underperforming, and investigating ways to
     fix it. If you're looking for ideas, you can head to
     [gitlab](https://gitlab.torproject.org) our bug tracking tool and look for
     tickets that have received the "First Contribution" label: these are ones
     that developers
     think would be pretty simple for a new person to work on.  For a bigger
     challenge, you might want to look for tickets with the "Project Ideas"
     keyword: these are tickets that the developers think might be a
     good idea to build, but which we have no time to work on any time
     soon.

     Or you might find another open ticket that piques your
     interest. It's all fine!

     For your first patch, it is probably NOT a good idea to make
     something huge or invasive.  In particular, you should probably
     avoid:

       * Major changes spread across many parts of the codebase.
       * Major changes to programming practice or coding style.
       * Huge new features or protocol changes.

  4. Meet the developers!

     We discuss stuff on the tor-dev mailing list and on the `#tor-dev`
     IRC channel on OFTC.  We're generally friendly and approachable,
     and we like to talk about how Tor fits together.  If we have ideas
     about how something should be implemented, we'll be happy to share
     them.

     We currently have a patch workshop at least once a week, where
     people share patches they've made and discuss how to make them
     better.  The time might change in the future, but generally,
     there's no bad time to talk, and ask us about patch ideas.

  5. Do you need to write a design proposal?

     If your idea is very large, or it will require a change to Tor's
     protocols, there needs to be a written design proposal before it
     can be merged. (We use this process to manage changes in the
     protocols.)  To write one, see the instructions at
     [the Tor proposal process](https://gitweb.torproject.org/torspec.git/plain/proposals/001-process.txt).
     If you'd like help writing a proposal, just ask!  We're happy to
     help out with good ideas.

     You might also like to look around the rest of that directory, to
     see more about open and past proposed changes to Tor's behavior.

  6. Writing your patch

     As you write your code, you'll probably want it to fit in with the
     standards of the rest of the Tor codebase so it will be easy for us
     to review and merge.  You can learn our coding standards in
     `doc/HACKING` directory.

     If your patch is large and/or is divided into multiple logical
     components, remember to divide it into a series of Git commits.  A
     series of small changes is much easier to review than one big lump.

  7. Testing your patch

     We prefer that all new or modified code have unit tests for it to
     ensure that it runs correctly.  Also, all code should actually be
     _run_ by somebody, to make sure it works.

     See `doc/HACKING/WritingTests.md` for more information on how we test things
     in Tor.  If you'd like any help writing tests, just ask!  We're
     glad to help out.

  8. Submitting your patch

     We review patches through tickets on our bugtracker at
     [gitlab](https://gitlab.torproject.org). You can either upload your patches there, or
     put them at a public git repository somewhere we can fetch them
     (like gitlab, github or bitbucket) and then paste a link on the appropriate
     ticket.

     Once your patches are available, write a short explanation of what
     you've done on trac, and then change the status of the ticket to
     needs_review.

  9. Review, Revision, and Merge

     With any luck, somebody will review your patch soon!  If not, you
     can ask on the IRC channel; sometimes we get really busy and take
     longer than we should.  But don't let us slow you down: you're the
     one who's offering help here, and we should respect your time and
     contributions.

     When your patch is reviewed, one of these things will happen:

       * The reviewer will say "_looks good to me_" and your
         patch will get merged right into Tor.  [Assuming we're not
         in the middle of a code-freeze window.  If the codebase is
         frozen, your patch will go into the next release series.]

       * OR the reviewer will say "_looks good, just needs some small
         changes!_"  And then the reviewer will make those changes,
         and merge the modified patch into Tor.

       * OR the reviewer will say "_Here are some questions and
         comments,_" followed by a bunch of stuff that the reviewer
         thinks should change in your code, or questions that the
         reviewer has.

         At this point, you might want to make the requested changes
         yourself, and comment on the trac ticket once you have done
         so. Or if you disagree with any of the comments, you should
         say so!  And if you won't have time to make some of the
         changes, you should say that too, so that other developers
         will be able to pick up the unfinished portion.

    Congratulations!  You have now written your first patch, and gotten
    it integrated into mainline Tor.
