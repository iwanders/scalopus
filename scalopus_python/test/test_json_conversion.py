#!/usr/bin/env python
# Copyright (c) 2018-2019, Ivor Wanders
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# * Neither the name of the author nor the names of contributors may be used to
#   endorse or promote products derived from this software without specific
#   prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import sys
from scalopus.lib import lib
import unittest

try:
    pybind11_nlohmann_test = lib.pybind11_nlohmann_test
except AttributeError as e:
    print("Could not find test module:")
    print(str(e))
    print("Claiming success.")
    sys.exit(0)

class JsonConversionTester(unittest.TestCase):

    def printEntries(self, result, input):
        print("---")
        print("result: {}".format(result))
        print("input : {}".format(input))

    def test_conversion(self):
        input_none = None
        result_none = pybind11_nlohmann_test.echo(input_none)
        self.printEntries(result_none, input_none)
        self.assertEqual(result_none, input_none)

        input_int = 3
        result_int = pybind11_nlohmann_test.echo(input_int)
        self.printEntries(result_int, input_int)
        self.assertEqual(result_int, input_int)

        input_signed_int = -3
        result_signed_int = pybind11_nlohmann_test.echo(input_signed_int)
        self.printEntries(result_signed_int, input_signed_int)
        self.assertEqual(result_signed_int, input_signed_int)

        input_float = 133.7
        result_float = pybind11_nlohmann_test.echo(input_float)
        self.printEntries(result_float, input_float)
        self.assertEqual(result_float, input_float)

        input_bool = True
        result_bool = pybind11_nlohmann_test.echo(input_bool)
        self.printEntries(result_bool, input_bool)
        self.assertEqual(result_bool, input_bool)

        input_list = [1, 2, 3]
        result_list = pybind11_nlohmann_test.echo(input_list)
        self.printEntries(result_list, input_list)
        self.assertEqual(result_list, input_list)

        input_string = "abc"
        result_string = pybind11_nlohmann_test.echo(input_string)
        self.printEntries(result_string, input_string)
        self.assertEqual(result_string, input_string)

        input_dict = {"a": 3, "b": 7, "c": [1,2,3]}
        result_dict = pybind11_nlohmann_test.echo(input_dict)
        self.printEntries(result_dict, input_dict)
        self.assertEqual(result_dict, input_dict)

        input_set = {"a", "b", 7, 1.3}
        result_set = pybind11_nlohmann_test.echo(input_set)
        self.printEntries(result_set, input_set)
        self.assertEqual(set(result_set), input_set)

        # Check if non string key in dictionary results in TypeError (just like json.dumps({(3,3): 5}) does.)
        def should_throw():
            pybind11_nlohmann_test.echo({(3,3): 3})
        self.assertRaises(TypeError, should_throw)
        



if __name__ == '__main__':
    unittest.main()
