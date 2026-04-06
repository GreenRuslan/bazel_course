import sys

def main():
    if len(sys.argv) != 3:
        print("Usage: generator.py <input> <output>")
        sys.exit(1)

    output_path = sys.argv[2]
    with open(output_path, "w") as f:
        f.write('// Auto-generated config\n#define GENERATED_VERSION "1.0"\n')

if __name__ == "__main__":
    main()
