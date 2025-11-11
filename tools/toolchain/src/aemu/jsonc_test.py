# -*- coding: utf-8 -*-
# Copyright 2025 - The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
import io
import unittest
from aemu.jsonc import load


class TestJsoncLoader(unittest.TestCase):
    def test_load_with_single_line_comments(self):
        """Test that single-line comments are stripped correctly."""
        jsonc_content = """
        {
            // This is a single-line comment
            "key": "value", // Another comment
            "number": 123
        }
        """
        fp = io.StringIO(jsonc_content)
        data = load(fp)
        self.assertEqual(data, {"key": "value", "number": 123})

    def test_load_with_multi_line_comments(self):
        """Test that multi-line comments are stripped correctly."""
        jsonc_content = """
        {
            /* This is a
             * multi-line comment */
            "key": "value",
            "number": 456 /* Another multi-line comment */
        }
        """
        fp = io.StringIO(jsonc_content)
        data = load(fp)
        self.assertEqual(data, {"key": "value", "number": 456})

    def test_load_with_mixed_comments(self):
        """Test that both single-line and multi-line comments are stripped."""
        jsonc_content = """
        {
            // Single-line comment
            "key1": "value1",
            /* Multi-line comment */
            "key2": "value2" // Trailing comment
        }
        """
        fp = io.StringIO(jsonc_content)
        data = load(fp)
        self.assertEqual(data, {"key1": "value1", "key2": "value2"})

    def test_load_no_comments(self):
        """Test that a regular JSON file is parsed correctly."""
        json_content = '{"key": "value", "number": 789}'
        fp = io.StringIO(json_content)
        data = load(fp)
        self.assertEqual(data, {"key": "value", "number": 789})

    def test_load_empty_json(self):
        """Test that an empty JSON object is parsed correctly."""
        json_content = "{}"
        fp = io.StringIO(json_content)
        data = load(fp)
        self.assertEqual(data, {})


if __name__ == "__main__":
    unittest.main()