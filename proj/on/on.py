import sys
import pathlib
from consts import *
import OracleServer

def main(argc: int, argv: list[str]):
    if argc != 2:
        print(f"Usage: {argv[0]} [configuration-file]")
        return 1
    file = pathlib.Path(argv[1])
    
    with OracleServer.OracleServer(HOST, PORT, file) as server:
        server.run()

if __name__ == '__main__':
    exit(main(len(sys.argv), sys.argv))
