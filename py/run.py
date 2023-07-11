from wavtools import wav_file_info


def run():
    with open('song.wav', 'rb') as f:
        data = f.read(100)
    wav_file_info(data)


if __name__ == '__main__':
    run()
