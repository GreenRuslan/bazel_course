import sys

# Importing from our utility module.
from utils.format import format_greeting

def main():
    print("Starting client CLI...")
    args = sys.argv[1:]
    
    if not args:
        name = "bazel user"
    else:
        name = " ".join(args)

    result = format_greeting(name)
    print(result)

if __name__ == "__main__":
    main()
