
import csv
import math
import matplotlib
import numpy as np
import sys
from PIL import Image

matplotlib.use('Agg')
import matplotlib.pyplot as plt

# local modules
sys.path.append('./programs/')
import tools.mystd as std
from TrackingFromCSV import Tracking


def main():
    # グラフの描画範囲(±[μm])
    graph_range = 2

    # カテゴリディレクトリのリストを取得
    csv_cat_list = std.get_dir_list(csv_path)
    pic_cat_list = std.get_dir_list(pic_path)
    print(csv_cat_list)
    print(pic_cat_list)

    # 実行時取得用のトラッキングクラスの生成
    temp_obj = Tracking()
    dt = temp_obj.dt

    # 全カテゴリを処理
    all_route = []
    all_frames = []
    all_real = []
    all_pix = []
    for csv_cat_name, pic_cat_name in zip(csv_cat_list, pic_cat_list):
        # サンプルディレクトリのリストを取得
        csv_cat_path = csv_path + csv_cat_name + '/'
        pic_cat_path = pic_path + pic_cat_name + '/'
        csv_smp_list = std.get_dir_list(csv_cat_path)
        pic_smp_list = std.get_dir_list(pic_cat_path)

        # 全サンプルを処理
        cat_route = []
        cat_frames = []
        cat_real = []
        cat_pix = []
        loop_flag = False
        for csv_smp_name, pic_smp_name in zip(csv_smp_list, pic_smp_list):
            tgt_csv_path = csv_cat_path + csv_smp_name + '/'
            tgt_pic_path = pic_cat_path + pic_smp_name + '/%05d.' + pic_ext
            output_path = out_path + dt + '/' + csv_cat_name + '/'
            # 出力先を作成
            std.make_dir(output_path)

            # トラッキングクラスの生成
            obj = Tracking()
            obj.set_datetime(dt)
            obj.set_in_path(tgt_csv_path, tgt_pic_path)
            obj.set_out_path(None, output_path, out_sub=csv_smp_name)
            # ログ表示ON/OFF
            # obj.set_log_print()
            # 動画用画像ON/OFF
            obj.movie_out()
            # マッチングの接続範囲(距離)
            obj.set_range(matching_range=10)
            # 経路の使用範囲
            obj.set_threshold(threshold=10)
            # 1ピクセルあたりの実際の距離
            pic_width = np.array(Image.open(tgt_pic_path % 0)).shape[0]
            pix2m = 6 / pic_width
            obj.set_pix2m(pix2m=pix2m)
            # グラフの描画範囲(±[μm])
            obj.set_graph_range(graph_range=graph_range)

            frames, route_list, moved_point_list, real_scale_list = obj.tracking()

            if loop_flag is False:
                cat_real = real_scale_list
                cat_pix = moved_point_list
                loop_flag = True
            else:
                cat_real = np.vstack((cat_real, real_scale_list))
                cat_pix = np.vstack((cat_pix, moved_point_list))

        all_route.append(cat_route)
        all_frames.append(cat_frames)
        all_real.append(cat_real)
        all_pix.append(cat_pix)

    # 移動距離計算とランキング
    print('\nAll trajectories drawing now...')
    all_dist = []
    for cat_name, cat_real in zip(csv_cat_list, all_real):
        # 全体リストの作成
        cat_traj = [traj for traj in cat_real]
        # 距離リストの作成
        cat_dist = []
        for traj in cat_traj:
            sum_dist = 0
            for i in range(1, len(traj)):
                x_1, y_1 = traj[i-1]
                x_2, y_2 = traj[i]
                sum_dist += math.sqrt((x_2 - x_1)**2 + (y_2 - y_1)**2)

            cat_dist.append(sum_dist)
        all_dist = all_dist + cat_dist
        print('>>> {} distance list saving ..'.format(cat_name))
        filename = out_path + dt + '/{}_dist.csv'.format(cat_name)
        np.savetxt(filename, np.array(cat_dist), delimiter=',')
    print(np.array(all_dist).shape)
    # all_dist_pix = []
    # for cat_name, cat_pix in zip(csv_cat_list, all_pix):
    #     # 全体リストの作成
    #     cat_traj = [traj for traj in cat_pix]
    #     # 距離リストの作成
    #     cat_dist = []
    #     for traj in cat_traj:
    #         sum_dist = 0
    #         for i in range(1, len(traj)):
    #             x_1, y_1 = traj[i-1]
    #             x_2, y_2 = traj[i]
    #             sum_dist += math.sqrt((x_2 - x_1)**2 + (y_2 - y_1)**2)

    #         cat_dist.append(sum_dist*(6/pic_width))
    #     all_dist_pix = all_dist_pix + cat_dist
    #     print('>>> {} pixel base distance list saving ..'.format(cat_name))
    #     filename = out_path + dt + '/{}_dist_pix.csv'.format(cat_name)
    #     np.savetxt(filename, np.array(cat_dist), delimiter=',')

    # ソート
    print('> Sorting ...')
    sorted_index = np.argsort(np.array(all_dist))[::-1]
    sorted_all_dist = np.array([all_dist[idx] for idx in sorted_index])
    # sorted_all_pix = np.array([all_dist_pix[idx] for idx in sorted_index])
    # sorted_all_trajectory = [all_traj[idx] for idx in sorted_index]

    # ランキング分割
    print('> Splitting ...')
    # sorted_all_trajectory = np.array(sorted_all_trajectory)
    # indices = [int(sorted_index.size * n) for n in [0.2, 0.2 + 0.4]] # 比率割り
    # print(indices)
    # exit()

    # 閾値割り
    high_th_idx = np.array(np.where(sorted_all_dist >= 2.0))
    mid_th_idx = np.array(np.where((sorted_all_dist >= 1.0) & (sorted_all_dist < 2.0)))
    low_th_idx = np.array(np.where(sorted_all_dist < 1.0))
    print('high:', high_th_idx.min(), high_th_idx.max())
    print('high:', sorted_all_dist[high_th_idx.min()], sorted_all_dist[high_th_idx.max()])
    print('mid:', mid_th_idx.min(), mid_th_idx.max())
    print('mid:', sorted_all_dist[mid_th_idx.min()], sorted_all_dist[mid_th_idx.max()])
    print('low:', low_th_idx.min(), low_th_idx.max())
    print('low:', sorted_all_dist[low_th_idx.min()], sorted_all_dist[low_th_idx.max()])
    indices = np.array([mid_th_idx.min(), low_th_idx.min()])

    # high, middle, low = np.split(sorted_all_trajectory, indices)
    high_idx, mid_idx, low_idx = np.split(sorted_index, indices)
    high_dists, mid_dists, low_dists = np.split(sorted_all_dist, indices)
    # high_pix, mid_pix, low_pix = np.split(sorted_all_pix, indices)
    print(high_idx.shape, mid_idx.shape, low_idx.shape)
    print(high_dists.shape, mid_dists.shape, low_dists.shape)
    print('High:', high_dists.min(), high_dists.max())
    print('Middle:', mid_dists.min(), mid_dists.max())
    print('Low:', low_dists.min(), low_dists.max())
    # print('High:', high_pix.min(), high_pix.max())
    # print('Middle:', mid_pix.min(), mid_pix.max())
    # print('Low:', low_pix.min(), low_pix.max())

    # インデックス整理
    print('> Index Shaping ... ')
    # 区切り番号
    size_list = [len(cat_real) for cat_real in all_real]
    cat_idx_list = []
    for i, size in enumerate(size_list):
        if i is 0:
            cat_idx_list.append(size)
        else:
            cat_idx_list.append(cat_idx_list[i-1]+size)

    prev_num = 0
    cat_rank_list = []
    for cat_num in cat_idx_list:
        cat_high = high_idx[(prev_num <= high_idx) & (high_idx < cat_num)]
        cat_mid = mid_idx[(prev_num <= mid_idx) & (mid_idx < cat_num)]
        cat_low = low_idx[(prev_num <= low_idx) & (low_idx < cat_num)]
        cat_list = []
        cat_list.append(cat_high)
        cat_list.append(cat_mid)
        cat_list.append(cat_low)
        cat_rank_list.append(cat_list)
        prev_num = cat_num

    # 全体描画
    print('> Drawing ...')
    cat_no = 0
    for cat_name, rank_list in zip(csv_cat_list, cat_rank_list):
        print('>>> {} graph drawing ..'.format(cat_name))
        # グラフの生成
        fig = plt.figure(figsize=(5, 5))
        ax = fig.add_subplot(1, 1, 1)

        # 上位描画
        print('>>> high: {}'.format(len(rank_list[0])))
        for route_idx in rank_list[0]:
            tgt_traj = all_real[cat_no][route_idx-cat_idx_list[cat_no]]
            for i in range(1, len(tgt_traj)):
                parent_x = tgt_traj[i-1][0]
                parent_y = -tgt_traj[i-1][1]
                child_x = tgt_traj[i][0]
                child_y = -tgt_traj[i][1]
                # 注意：画像の見え方と実際の値の齟齬を解消するために，y軸は符号反転する
                ax.plot(
                    [parent_x, child_x], [parent_y, child_y],
                    c='r')

        # 中位描画
        print('>>>  mid: {}'.format(len(rank_list[1])))
        for route_idx in rank_list[1]:
            tgt_traj = all_real[cat_no][route_idx-cat_idx_list[cat_no]]
            for i in range(1, len(tgt_traj)):
                parent_x = tgt_traj[i-1][0]
                parent_y = -tgt_traj[i-1][1]
                child_x = tgt_traj[i][0]
                child_y = -tgt_traj[i][1]
                # 注意：画像の見え方と実際の値の齟齬を解消するために，y軸は符号反転する
                ax.plot(
                    [parent_x, child_x], [parent_y, child_y],
                    c='y')

        # 下位描画
        print('>>>  low: {}'.format(len(rank_list[2])))
        for route_idx in rank_list[2]:
            tgt_traj = all_real[cat_no][route_idx-cat_idx_list[cat_no]]
            for i in range(1, len(tgt_traj)):
                parent_x = tgt_traj[i-1][0]
                parent_y = -tgt_traj[i-1][1]
                child_x = tgt_traj[i][0]
                child_y = -tgt_traj[i][1]
                # 注意：画像の見え方と実際の値の齟齬を解消するために，y軸は符号反転する
                ax.plot(
                    [parent_x, child_x], [parent_y, child_y], c='b')

        # 全経路の最大値(+余白)を求めて，グラフ枠のサイズを決める
        # max_range = np.absolute(sorted_all_trajectory).max() + pix2m
        # plt.xlim([-max_range, max_range])
        # plt.ylim([-max_range, max_range])

        # グラフ枠のサイズを3umに設定する
        w = graph_range
        plt.xlim([-w, w])
        plt.ylim([-w, w])

        # グラフの保存
        print('>>> Saving ..')
        filename = out_path + dt + '/{}_traj.png'.format(cat_name)
        plt.savefig(filename, dpi=100, transparent=True)
        plt.close()

        # 個数情報の保存
        info_array = [
            ['High', len(rank_list[0])],
            # ['High min-max', high_dists.min(), high_dists.max()],
            ['Middle', len(rank_list[1])],
            # ['Middle min-max', mid_dists.min(), mid_dists.max()],
            ['Low', len(rank_list[2])],
            # ['Low min-max', low_dists.min(), low_dists.max()]
        ]
        filename = out_path + dt + '/{}_num.csv'.format(cat_name)
        with open(filename, 'w') as f:
            writer = csv.writer(f, lineterminator='\n')
            writer.writerows(info_array)

        # カウンタの加算
        cat_no += 1

    print('> Done.')


if __name__ == '__main__':
    # 入出力連続画像ディレクトリ指定
    csv_path = './output/findmaxima/'
    pic_path = './input/'
    out_path = './output/result/'
    pic_ext = 'png'

    print('csv In:' + csv_path)
    print('pic In:' + pic_path)
    print('output:' + out_path)

    main()
