import numpy as np
import os


def get_dir_list(super_path):
    # 指定ディレクトリ配下のディレクトリ名のリストを取得
    dir_list = []
    for item in os.listdir(super_path):
        if os.path.isdir(super_path + item):
            dir_list.append(item)
    return sorted(dir_list)


def get_file_list(dir_path, ext):
    # 指定ディレクトリ配下の指定拡張子のファイルのリストをソートして返却
    file_list = []
    for item in os.listdir(dir_path):
        if os.path.isfile(dir_path + item):
            file_name, file_ext = os.path.splitext(item)
            if file_ext == ('.' + ext):
                file_list.append(item)
    return sorted(file_list)


def make_dir(path):
    # 指定のパスのディレクトリが存在するか確認し，存在しなければ作成
    print('std >> dirctory making ...')
    if os.path.isdir(path):
        print('std >>> following directories were aleady exist.')
        print('std >>> PATH: ' + path)
    else:
        os.makedirs(path)
        print('std >>> made following directories:')
        print('std >>> PATH: ' + path)


def image_load(path_list, color='gray'):
    import cv2
    from tqdm import tqdm
    pr = std_print(label='std')
    pr.echo('image Loading.', level=1)
    images = []
    with tqdm(total=len(path_list), ascii=True, ncols=100) as pbar:
        for path in path_list:
            pbar.update(1)
            # 画像の読み込み
            if color == 'gray':
                img = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
            elif color == 'rgb':
                img = cv2.imread(path, cv2.IMREAD_COLOR)
            elif color == 'alpha':
                img = cv2.imread(path, cv2.IMREAD_UNCHANGED)
            else:
                pr.echo('!-- Image color setting error. --!', level=2)
                exit()
            images.append(img)
    if len(images) == 1:
        return images[0]
    else:
        return images


def file_counter(file_path, reset=False):
    if (reset is False) and (os.path.isfile(file_path)):
        print('std >> open counter txt file.')
        with open(file_path, mode='r') as f:
            s = f.read()
        with open(file_path, mode='w') as f:
            next_c = int(s) + 1
            f.write(str(next_c))
    else:
        print('std >> make new counter txt file.')
        with open(file_path, mode='w') as f:
            f.write('2')
            s = '1'
    return int(s)


class Normalize:
    def __init__(self):
        pass

    def min_max(self, data):
        data = np.array(data)
        _max = data.max()
        _min = data.min()
        return (data - _min) / (_max - _min)

    def specify_min_max(self, data, spec_min, spec_max):
        data = np.array(data)
        _max = data.max()
        _min = data.min()
        return (data-_min) * (spec_max-spec_min) / (_max-_min) + spec_min


class std_print:
    def __init__(self, label='std'):
        self.label = label

    def echo(self, string, level=1):
        if level == 0:
            pass
        else:
            print('[{}]'.format(self.label), end=' ')
            for i in range(level):
                print('>>>', end=' ')
            print(string)
