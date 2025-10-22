Easy-RSA 3 GitHub Contributions Documentation
=============================================

This document explains how to contribute to Easy-RSA 3.

Please follow these simple steps and make contributing easier.

Intended audience: Everyone.

Contributing Guide
------------------

-   **Do not** edit Easy-RSA `master` branch.

-   **Do not** edit Easy-RSA `master` branch.

    Pull Requests submitted from `master` branch may be squashed or rejected.

### Create a new branch:

-   Select a suitable name for the new branch. eg: `doc-contrib-typo`

    ```
    git checkout -b doc-contrib-typo
    ```

-   Make changes to the new branch.

    Please use tabs to indent the code but only use tabs at the beginning of
    the line.

-   Review the changes:

    ```
    git diff
    ```

-   Stage the changes:

    ```
    git add -A
    ```

-   Show the extent of the changes:

    ```
    git status -v
    ```

-   Commit the changes:

    ```
    git commit -sS
    ```

    Please write a detailed commit message.

    github `help` has details of creating a private key.

    Using github `no-reply` email address is suitable for the `Signed-off-by:`
    line.

-   Push the changes:

    ```
    git push origin doc-contrib-typo
    ```

-   Share the changes:

    ```
    Raise a Pull Request on github.
    ```

Keeping your fork synchronised
-----------------------------

-   Configure the `upstream` remote for your fork:

    ```
    git remote add upstream https://github.com/OpenVPN/easy-rsa.git
    ```

-   Verify the remote sources:

    ```
    git remote -v
    ```

    Remote `origin` will have **your** repository:

    ```
    origin https://github.com/TinCanTech/easy-rsa.git (fetch)
    origin https://github.com/TinCanTech/easy-rsa.git (push)
    ```

    Remote `upstream` will be `Openvpn/easy-rsa`:

    ```
    upstream https://github.com/Openvpn/easy-rsa.git (fetch)
    upstream https://github.com/Openvpn/easy-rsa.git (push)
    ```

### Synchronising your fork:

-   Select `master` branch:

    ```
    git checkout master
    ```

-   Fetch changes in `upstream`:

    ```
    git fetch upstream
    ```

-   Merge changes in `upstream`:

    ```
    git merge upstream/master
    ```

-   Update your fork on github:

    ```
    git push
    ```


    Your fork is now synchronised.
