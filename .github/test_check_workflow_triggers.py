#!/usr/bin/env python3
"""Tests for check_workflow_triggers.py; run: python3 .github/test_check_workflow_triggers.py"""
import contextlib
import importlib.util
import io
import os
import tempfile
import unittest
from unittest import mock

_HERE = os.path.dirname(os.path.abspath(__file__))
_SPEC = importlib.util.spec_from_file_location(
    'check_workflow_triggers', os.path.join(_HERE, 'check_workflow_triggers.py'))
cwt = importlib.util.module_from_spec(_SPEC)
_SPEC.loader.exec_module(cwt)


class CheckWorkflowTriggers(unittest.TestCase):

    def run_check(self, branches_yaml, existing=('master',), env=None):
        with tempfile.TemporaryDirectory() as where:
            with open(os.path.join(where, 'w.yml'), 'w', encoding='utf-8') as f:
                f.write('on:\n  push:\n    branches: %s\n' % branches_yaml)
            with mock.patch.object(cwt, 'existing_branches', lambda: set(existing)), \
                 mock.patch.object(cwt, 'default_branch', lambda: 'master'), \
                 mock.patch.dict(os.environ, env or {}, clear=False), \
                 contextlib.redirect_stdout(io.StringIO()):
                return cwt.main(['check', where])

    def test_existing_literal_branch_passes(self):
        self.assertEqual(self.run_check('[ master ]'), 0)

    def test_missing_literal_branch_fails(self):
        self.assertEqual(self.run_check('[ main ]'), 1)

    def test_glob_is_not_judged(self):
        self.assertEqual(self.run_check("[ 'release-*' ]"), 0)

    def test_bare_string_is_one_branch(self):
        self.assertEqual(self.run_check('master'), 0)

    def test_unquoted_non_string_entry_fails(self):
        self.assertEqual(self.run_check('[ on ]'), 1)
        self.assertEqual(self.run_check('[ 3.0 ]'), 1)

    def test_exclusion_does_not_hide_missing_branch(self):
        self.assertEqual(self.run_check("[ main, '!wip' ]"), 1)

    def test_only_exclusions_fails(self):
        self.assertEqual(self.run_check("[ '!wip' ]"), 1)

    def test_no_branches_found_fails_in_ci(self):
        self.assertEqual(self.run_check('[ master ]', existing=(), env={'CI': 'true'}), 1)


if __name__ == '__main__':
    unittest.main()
