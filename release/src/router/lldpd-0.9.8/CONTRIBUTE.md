Coding standards
----------------

Contributed code should roughly follow [OpenBSD style][1]. For Emacs,
I am using the following snippet to get the appropriate indentation:

    (c-add-style
     "openbsd"
     '("bsd"
       (c-basic-offset . 8)
       (c-tab-width . 8)
       (fill-column . 80)
       (indent-tabs-mode . t)
       (c-offsets-alist . ((defun-block-intro     . +)
                           (statement-block-intro . +)
                           (statement-case-intro  . +)
                           (statement-cont        . *)
                           (substatement-open     . *)
                           (substatement          . +)
                           (arglist-cont-nonempty . *)
                           (inclass               . +)
    		       (inextern-lang         . 0)
                           (knr-argdecl-intro     . +)))))

Important stuff is to use tabulations. Each tabulation has a width of
8 characters. This limits excessive nesting. Try to respect the 80
columns limit if possible.

Opening braces are on the same line, except for functions where they
are on their own lines. Closing braces are always on their own
lives. Return type for functions are on their own lines too:

    int
    main(int argc, char *argv[])
    {
        /* [...] */
    }

[1]: http://www.openbsd.org/cgi-bin/man.cgi?query=style&sektion=9

Submitting patches
------------------

Patches against git tip are preferred. Please, clone the repository:

    git clone https://github.com/vincentbernat/lldpd.git

Do any modification you need. Commit the modifications with a
meaningful message:

 1. Use a descriptive first-line.

 2. Prepend the first line with the name of the subsystem modified
    (`lldpd`, `lib`, `client`) or something a bit more precise.

 3. Don't be afraid to put a lot of details in the commit message.

Use `git format-patch` to get patches to submit:

    git format-patch origin/master

Feel free to use `git send-email` which is like `git format-patch` but
will propose to directly send patches by email. You can also open a
[pull request][2] on Github.

[2]: https://help.github.com/articles/using-pull-requests
