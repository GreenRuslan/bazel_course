import unittest
from utils.format import format_greeting

class ClientTest(unittest.TestCase):
    def test_format_greeting_normal(self):
        # Using unittest module for simple python test
        result = format_greeting("bazel builder")
        self.assertEqual(result, "Hello, Bazel Builder!")

    def test_format_greeting_empty(self):
        result = format_greeting("")
        self.assertEqual(result, "Hello, Friend!")

if __name__ == '__main__':
    unittest.main()
