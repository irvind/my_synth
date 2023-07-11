import struct


def ascii_nums_to_str(num_li):
    return ''.join([chr(ch) for ch in num_li])


def wav_file_info(file):
    result = struct.unpack('<bbbb' ,file[0:4])
    assert ascii_nums_to_str(result) == 'RIFF'

    file_size = struct.unpack('<i', file[4:8])
    print('file_size:', file_size)

    result = struct.unpack('<bbbb' ,file[8:12])
    assert ascii_nums_to_str(result) == 'WAVE'

    block_chars = struct.unpack('<bbbb' ,file[12:16])
    block_str = ascii_nums_to_str(block_chars)
    print('block:', block_str)
    if block_str == 'JUNK':
        skip_bytes = struct.unpack('<i', file[16:20])[0]
        print(skip_bytes)

    print(file[20+skip_bytes:20+skip_bytes+24])
