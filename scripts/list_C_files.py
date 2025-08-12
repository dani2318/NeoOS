import os 
import sys

def list_c_files(root_dir):
    for dirpath, _, filenames in os.walk(root_dir):
        for f in filenames:
            if f.endswith('.c'):
                print(os.path.join(dirpath, f))


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <root_dir>")
        sys.exit(1)
    root = sys.argv[1]
    list_c_files(root)