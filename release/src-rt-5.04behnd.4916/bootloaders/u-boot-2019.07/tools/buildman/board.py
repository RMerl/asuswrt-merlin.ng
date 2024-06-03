# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2012 The Chromium OS Authors.

import re

class Expr:
    """A single regular expression for matching boards to build"""

    def __init__(self, expr):
        """Set up a new Expr object.

        Args:
            expr: String cotaining regular expression to store
        """
        self._expr = expr
        self._re = re.compile(expr)

    def Matches(self, props):
        """Check if any of the properties match the regular expression.

        Args:
           props: List of properties to check
        Returns:
           True if any of the properties match the regular expression
        """
        for prop in props:
            if self._re.match(prop):
                return True
        return False

    def __str__(self):
        return self._expr

class Term:
    """A list of expressions each of which must match with properties.

    This provides a list of 'AND' expressions, meaning that each must
    match the board properties for that board to be built.
    """
    def __init__(self):
        self._expr_list = []
        self._board_count = 0

    def AddExpr(self, expr):
        """Add an Expr object to the list to check.

        Args:
            expr: New Expr object to add to the list of those that must
                  match for a board to be built.
        """
        self._expr_list.append(Expr(expr))

    def __str__(self):
        """Return some sort of useful string describing the term"""
        return '&'.join([str(expr) for expr in self._expr_list])

    def Matches(self, props):
        """Check if any of the properties match this term

        Each of the expressions in the term is checked. All must match.

        Args:
           props: List of properties to check
        Returns:
           True if all of the expressions in the Term match, else False
        """
        for expr in self._expr_list:
            if not expr.Matches(props):
                return False
        return True

class Board:
    """A particular board that we can build"""
    def __init__(self, status, arch, cpu, soc, vendor, board_name, target, options):
        """Create a new board type.

        Args:
            status: define whether the board is 'Active' or 'Orphaned'
            arch: Architecture name (e.g. arm)
            cpu: Cpu name (e.g. arm1136)
            soc: Name of SOC, or '' if none (e.g. mx31)
            vendor: Name of vendor (e.g. armltd)
            board_name: Name of board (e.g. integrator)
            target: Target name (use make <target>_defconfig to configure)
            options: board-specific options (e.g. integratorcp:CM1136)
        """
        self.target = target
        self.arch = arch
        self.cpu = cpu
        self.board_name = board_name
        self.vendor = vendor
        self.soc = soc
        self.options = options
        self.props = [self.target, self.arch, self.cpu, self.board_name,
                      self.vendor, self.soc, self.options]
        self.build_it = False


class Boards:
    """Manage a list of boards."""
    def __init__(self):
        # Use a simple list here, sinc OrderedDict requires Python 2.7
        self._boards = []

    def AddBoard(self, board):
        """Add a new board to the list.

        The board's target member must not already exist in the board list.

        Args:
            board: board to add
        """
        self._boards.append(board)

    def ReadBoards(self, fname):
        """Read a list of boards from a board file.

        Create a board object for each and add it to our _boards list.

        Args:
            fname: Filename of boards.cfg file
        """
        with open(fname, 'r') as fd:
            for line in fd:
                if line[0] == '#':
                    continue
                fields = line.split()
                if not fields:
                    continue
                for upto in range(len(fields)):
                    if fields[upto] == '-':
                        fields[upto] = ''
                while len(fields) < 8:
                    fields.append('')
                if len(fields) > 8:
                    fields = fields[:8]

                board = Board(*fields)
                self.AddBoard(board)


    def GetList(self):
        """Return a list of available boards.

        Returns:
            List of Board objects
        """
        return self._boards

    def GetDict(self):
        """Build a dictionary containing all the boards.

        Returns:
            Dictionary:
                key is board.target
                value is board
        """
        board_dict = {}
        for board in self._boards:
            board_dict[board.target] = board
        return board_dict

    def GetSelectedDict(self):
        """Return a dictionary containing the selected boards

        Returns:
            List of Board objects that are marked selected
        """
        board_dict = {}
        for board in self._boards:
            if board.build_it:
                board_dict[board.target] = board
        return board_dict

    def GetSelected(self):
        """Return a list of selected boards

        Returns:
            List of Board objects that are marked selected
        """
        return [board for board in self._boards if board.build_it]

    def GetSelectedNames(self):
        """Return a list of selected boards

        Returns:
            List of board names that are marked selected
        """
        return [board.target for board in self._boards if board.build_it]

    def _BuildTerms(self, args):
        """Convert command line arguments to a list of terms.

        This deals with parsing of the arguments. It handles the '&'
        operator, which joins several expressions into a single Term.

        For example:
            ['arm & freescale sandbox', 'tegra']

        will produce 3 Terms containing expressions as follows:
            arm, freescale
            sandbox
            tegra

        The first Term has two expressions, both of which must match for
        a board to be selected.

        Args:
            args: List of command line arguments
        Returns:
            A list of Term objects
        """
        syms = []
        for arg in args:
            for word in arg.split():
                sym_build = []
                for term in word.split('&'):
                    if term:
                        sym_build.append(term)
                    sym_build.append('&')
                syms += sym_build[:-1]
        terms = []
        term = None
        oper = None
        for sym in syms:
            if sym == '&':
                oper = sym
            elif oper:
                term.AddExpr(sym)
                oper = None
            else:
                if term:
                    terms.append(term)
                term = Term()
                term.AddExpr(sym)
        if term:
            terms.append(term)
        return terms

    def SelectBoards(self, args, exclude=[], boards=None):
        """Mark boards selected based on args

        Normally either boards (an explicit list of boards) or args (a list of
        terms to match against) is used. It is possible to specify both, in
        which case they are additive.

        If boards and args are both empty, all boards are selected.

        Args:
            args: List of strings specifying boards to include, either named,
                  or by their target, architecture, cpu, vendor or soc. If
                  empty, all boards are selected.
            exclude: List of boards to exclude, regardless of 'args'
            boards: List of boards to build

        Returns:
            Tuple
                Dictionary which holds the list of boards which were selected
                    due to each argument, arranged by argument.
                List of errors found
        """
        result = {}
        warnings = []
        terms = self._BuildTerms(args)

        result['all'] = []
        for term in terms:
            result[str(term)] = []

        exclude_list = []
        for expr in exclude:
            exclude_list.append(Expr(expr))

        found = []
        for board in self._boards:
            matching_term = None
            build_it = False
            if terms:
                match = False
                for term in terms:
                    if term.Matches(board.props):
                        matching_term = str(term)
                        build_it = True
                        break
            elif boards:
                if board.target in boards:
                    build_it = True
                    found.append(board.target)
            else:
                build_it = True

            # Check that it is not specifically excluded
            for expr in exclude_list:
                if expr.Matches(board.props):
                    build_it = False
                    break

            if build_it:
                board.build_it = True
                if matching_term:
                    result[matching_term].append(board.target)
                result['all'].append(board.target)

        if boards:
            remaining = set(boards) - set(found)
            if remaining:
                warnings.append('Boards not found: %s\n' % ', '.join(remaining))

        return result, warnings
