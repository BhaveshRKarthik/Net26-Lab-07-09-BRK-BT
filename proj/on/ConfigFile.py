import pathlib

def read_config(config_file: pathlib.Path, old_len: int | None):
    config_raw = []
    next_len: int = old_len - 1 if old_len is not None else -1
    
    with config_file.open() as file:
        for line in file:
            line = line.strip()

            if line == '' or line[0] == '#': continue # comment or blanks

            l = [int(x) for x in line.split()]
            
            assert next_len in { -1, len(l) }, "Incorrect number of integers in line"
            assert len(l) < 26, "Too many integer entries in line"
            assert all((x == -1 or x > 0) for x in l)

            config_raw.append(l)
            next_len = len(l) - 1

    assert next_len == 0, f"Incomplete config file"

    N = len(config_raw)
    config = [[0]*(N+1) for _ in range(N+1)]

    for i in range(N):
        for j in range(N-i):
            config[i][i+j+1] = config[i+j+1][i] = config_raw[i][j]

    return config
