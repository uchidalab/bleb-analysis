import os
import numpy as np
from tqdm import tqdm
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from PIL import Image


class OverlapGraph():
    # 指定の形式にトラッキング結果を整形して出力するクラス

    def __init__(self, data_list, route_list):
        # コンストラクタ

        # args:
        self.data_list = data_list
        self.route_list = route_list

        # maxrange : グラフの描画範囲(±[μm])
        self.maxrange = 3

        # その他
        self.all_node_list = []
        self.all_node_id = []
        self.devided_route = []
        self.colors = []

    def setMaxRange(self, value):
        self.maxrange = value

    def route_extractor(self, threshold):
        # トラッキング結果から閾値のフレーム連続してトラッキングされている経路を抽出

        # 有効経路の抽出
        effective_route = [
            route for route in self.route_list
            if route.route_length > threshold
        ]

        # 全Nodeクラスの連続配列を作る
        self.all_node_list = [node for data in self.data_list for node in data]
        self.all_node_id = [node.ID for node in self.all_node_list]

        # 経路始点から閾値までのノードIDとTIMEのリストを作る
        self.devided_route = [route.route[:threshold] for route in effective_route]
        self.devided_route_time = [
            [t for t in range(route.t, route.t+10)]
            for route in effective_route
        ]

        # ランダムに色を有効経路の個数分生成（値0～255の範囲でn行3列のランダムなndarrayを生成）
        self.colors = np.random.rand(len(effective_route), 3)

    def figout(self, out_path, *, pix2m=1):
        # 抽出した全経路を重ねあわせて表示する．

        print('\nRoute overlapping graph making now ...')

        # 出力先作成
        print('> Check output dirctory: \"' + out_path + '\"')
        if os.path.isdir(out_path) is False:
            print('> Output directory are not found.')
            os.makedirs(out_path, exist_ok=True)
            print('> New directory created!')
        else:
            print('> Already exist.')

        print('> Calculating ...')

        # 描画用の座標リストを作る
        point_list = [
            [
                [self.all_node_list[ID].x, self.all_node_list[ID].y]
                for ID in route
            ] for route in self.devided_route
        ]

        # 始点の座標を(0, 0)にして全体を平行移動する
        moved_point_list = [
            [
                np.array(np.array(node) - np.array(route[0])).tolist()
                for node in route
            ] for route in point_list
        ]

        # 数値を実際の距離に合わせる
        real_scale_list = np.array(moved_point_list) * pix2m

        # グラフの生成
        fig = plt.figure(figsize=(5, 5))
        ax = fig.add_subplot(1, 1, 1)

        for r_idx, route in enumerate(real_scale_list):
            for i in range(1, len(route)):
                parent_x = route[i-1][0]
                parent_y = -route[i-1][1]
                child_x = route[i][0]
                child_y = -route[i][1]
                # 注意：画像の見え方と実際の値の齟齬を解消するために，y軸は符号反転する
                ax.plot(
                    [parent_x, child_x], [parent_y, child_y],
                    c=self.colors[r_idx].tolist())

        # 全経路の最大値(+余白)を求めて，グラフ枠のサイズを決める
        # max_range = np.absolute(real_scale_list).max() + pix2m
        # plt.xlim([-max_range, max_range])
        # plt.ylim([-max_range, max_range])

        # グラフ枠のサイズを3umに設定する
        w = self.maxrange
        plt.xlim([-w, w])
        plt.ylim([-w, w])

        # グラフの保存
        filename = out_path + '/overlapped_route.png'
        plt.savefig(filename, dpi=100, transparent=True)
        plt.close()
        print('> Done.')

        return moved_point_list, real_scale_list

    def trajout(self, out_path, *, pix2m=1):
        # 抽出した全経路を個別に並べて出力する．

        print('\nRoute trajectory graph making now ...')

        # 出力先作成
        out_path = out_path + 'traj_fig/'
        print('> Check output dirctory: \"'+ out_path + '\"')
        if os.path.isdir(out_path) is False:
            print('> Output directory are not found.')
            os.makedirs(out_path, exist_ok=True)
            print('> New directory created!')
        else:
            print('> Already exist.')

        print('> Calculating ...')

        # 描画用の座標リストを作る
        point_list = [
            [
                [self.all_node_list[ID].x, self.all_node_list[ID].y]
                for ID in route
            ] for route in self.devided_route
        ]

        # 始点の座標を(0, 0)にして全体を平行移動する
        moved_point_list = [
            [
                np.array(np.array(node) - np.array(route[0])).tolist()
                for node in route
            ] for route in point_list
        ]

        # 数値を実際の距離に合わせる
        real_scale_list = np.array(moved_point_list) * pix2m

        # 全経路の最大値(+余白)を求めて，グラフ枠のサイズを決める
        max_range = np.absolute(real_scale_list).max() + pix2m

        # プログレスバーの生成
        with tqdm(total=len(real_scale_list), ascii=True, ncols=100) as pbar:
            for r_idx, route in enumerate(real_scale_list):
                # プログレスバーを進める
                pbar.update(1)

                # グラフの生成
                fig = plt.figure(figsize=(2, 2))
                ax = fig.add_subplot(1, 1, 1)

                for i in range(1, len(route)):
                    parent_x = route[i-1][0]
                    parent_y = -route[i-1][1]
                    child_x = route[i][0]
                    child_y = -route[i][1]
                    # 注意：画像の見え方と実際の値の齟齬を解消するために，y軸は符号反転する
                    ax.plot(
                        [parent_x, child_x], [parent_y, child_y],
                        c=self.colors[r_idx].tolist())

                # グラフの描画範囲の設定
                plt.xlim([-max_range, max_range])
                plt.ylim([-max_range, max_range])

                # グラフ枠のサイズを3umに設定する
                # w = self.maxrange
                # plt.xlim([-w, w])
                # plt.ylim([-w, w])

                # グラフの保存
                filename = out_path + '/route_%05d.png' % r_idx
                plt.savefig(filename, dpi=100, transparent=True)
                plt.close()
        print('> Done.')

    def movout(self, out_path, bg_path):
        # トラッキング結果をオーバーレイ描画して出力する

        print('\nRoute trajectory movie making now ...')

        # 出力先作成
        out_path = out_path + 'mov_fig/'
        print('> Check output dirctory: \"' + out_path + '\"')
        if os.path.isdir(out_path) is False:
            print('> Output directory are not found.')
            os.makedirs(out_path, exist_ok=True)
            print('> New directory created!')
        else:
            print('> Already exist.')

        print('> Calculating ...')

        # プログレスバーの生成
        with tqdm(total=len(self.data_list), ascii=True, ncols=100) as pbar:
            # フレームごとにループ処理する
            for idx, nodes in enumerate(self.data_list):
                # プログレスバーを進める
                pbar.update(1)

                # 背景の読み込み
                img = Image.open(bg_path % idx)
                img = np.array(img)
                h = img.shape[0]
                w = img.shape[1]

                # グラフの生成
                fig = plt.figure(figsize=(w/10, h/10))
                ax = fig.add_subplot(1, 1, 1)

                # 画像のカラー/グレースケール判定
                if len(img.shape) is 2:
                    ax.imshow(img, 'gray')
                    print('> input image is gray scale.')
                else:
                    ax.imshow(img)
                    # print('> input image is RGB color')

                # 描画用の座標リストを作る
                for r_idx, (T, route) in enumerate(zip(self.devided_route_time, self.devided_route)):
                    if idx in T:
                        index = T.index(idx)
                        for i in reversed(range(index+1)):
                            ID = route[i]
                            node = self.all_node_list[ID]
                            if node.parent_ID is not None:
                                parent = self.all_node_list[node.parent_ID]
                                child_x = node.x
                                child_y = node.y
                                parent_x = parent.x
                                parent_y = parent.y
                                ax.plot(
                                    [parent_x, child_x], [parent_y, child_y],
                                    c=self.colors[r_idx].tolist()
                                    )

                # ノードの描画
                for i, node in enumerate(nodes):
                    ax.scatter(node.x, node.y, c='red', s=200, marker='*', edgecolors='black', linewidths=1)

                # 範囲設定
                plt.xlim([0, w-1])
                plt.ylim([h-1, 0])

                filename = out_path + '%05d.tif' % idx
                plt.savefig(filename, dpi=100)
                plt.close()
        print('> Done.')
