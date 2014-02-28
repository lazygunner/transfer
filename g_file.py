import sys

def generate_file(file_name, size):
    file = open("files/" + file_name, "wb")
    base_num = 10000000
    if(size > 1):
        for i in range(1, size + 1):
            for j in range(1, 513):
                for k in range(1, 257):
                    file.write(('%08d' %(i * 1000 + j)))
    else:
        size = int(size * 512)
        for i in range(1, size):
            for k in range (1, 257):
                file.write(str(10000000 + i))

generate_file(sys.argv[1], int(sys.argv[2]))
