import numpy as np
import csv
import matplotlib.pyplot as plt
import os
import cv2
import itertools
import pandas as pd


date_list = ['Sample']
subdir_list = [range(1, 2)]
filedir = os.path.abspath(os.path.dirname(__file__))
loadpath1 = filedir + '/BlebRatio_FigsCreator_TricolorMap/result/'
loadpath2 = filedir + '/BlebRatio_FigsCreator_TricolorMap/input/'
output_path = filedir + '/result_histogram/'
if os.path.isdir(output_path) is False:
    os.makedirs(output_path)


def diff_time(subdir):
    src = cv2.imread('{0}/{1}/trans_TricolorMap.tif'.format(loadpath1, subdir))
    d_time = []
    for i in range(src.shape[1]):
        j = 0
        while True:
            if np.array_equal(src[j, i], [255, 255, 255]) is False:
                buff_color = src[j, i]
                start_time = j
                theta = i
                delta_time = 1
                while True:
                    j += 1
                    if j >= src.shape[0]:
                        j -= 1
                        break

                    if np.array_equal(src[j, i], buff_color) is False:
                        end_time = j
                        if buff_color[0] == 255: delta_time = [delta_time, start_time, end_time, theta, 'blue']
                        else: delta_time = [delta_time, start_time, end_time, theta, 'red']
                        d_time.append(delta_time)
                        j -= 1
                        break
                    delta_time += 1
            j += 1
            if j >= src.shape[0]: break

    d_time = np.asarray(d_time)
    up_time = d_time[d_time[:, -1] == 'red', :-1].astype(np.int)
    down_time = d_time[d_time[:, -1] == 'blue', :-1].astype(np.int)

    return up_time, down_time


def diff_distance(src, subdir):
    csv_input = pd.read_csv(filepath_or_buffer='{0}/{1}/distance.csv'.format(loadpath2, subdir),  encoding='ms932', sep=',')
    dis_map = csv_input.values[:, 1:]
    delta_dis = dis_map[src[:, 2], src[:, 3]] - dis_map[src[:, 1], src[:, 3]]
    velocity_list = delta_dis / src[:, 0]

    return delta_dis, velocity_list


def diff_dist_based_distmap(subdir):
    csv_input = pd.read_csv(filepath_or_buffer='{0}/{1}/distance.csv'.format(loadpath2, subdir),  encoding='ms932', sep=',')
    dis_map = csv_input.values[:, 1:]
    buff = [dis_map[i+1] - dis_map[i] for i in range(len(dis_map)-1)]
    abs_buff = [abs(x/50) for inner in buff for x in inner]
    buff = [x/50 for inner in buff for x in inner]
    return buff, abs_buff


def evaluate_func(d, k):
    velo_list = []
    max_velo = 0
    min_velo = np.inf
    for i in k:
        dirs = '{0}/{1}'.format(d, i)
        diff_dist, _ = diff_dist_based_distmap(dirs)
        velo_list.append(diff_dist)
        if max_velo < max(diff_dist): max_velo = max(diff_dist)
        if min_velo > min(diff_dist): min_velo = min(diff_dist)
    return velo_list, max_velo, min_velo
    

def main():
    velo_list = []
    max_velo = 0
    min_velo = np.inf
    for d, k in zip(date_list, subdir_list):
        dst, buff1, buff2 = evaluate_func(d, k)
        for i in dst:
            velo_list.append(i)
        if max_velo < buff1: max_velo = buff1
        if min_velo > buff2: min_velo = buff2

    bin = 200
    dst = np.empty((0, bin))
    for i in velo_list:
        velo_hist = plt.hist(i, range=(-2, 2), bins=bin)
        dst = np.append(dst, np.array(velo_hist[0])[None, :], axis=0)
        plt.close()
    with open('{0}/result_histogram.csv'.format(output_path, d), 'w') as f:
        writer = csv.writer(f, lineterminator='\n')
        dst = np.append(velo_hist[1][None, :-1], dst, axis=0)
        writer.writerows(dst.T)
    

if __name__ == '__main__':
    main()