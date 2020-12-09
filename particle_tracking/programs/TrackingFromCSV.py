import os
import csv

# 自作 =====
from stableMarriage.GaleShapley import GaleShapley
from stableMarriage.Node import Node
from stableMarriage.Route import Route
# from AnalysisTools.tracking2pic import overlay
# from AnalysisTools.tracking2pic import overlayWithOrigin
from AnalysisTools.OverlapGraph import OverlapGraph
# from MSD import meanSquareDisplacement
from tools.dateGetter import get_datetime


class Tracking(object):

    def __init__(self):
        # コンストラクタ

        # 実行日時の取得
        self.dt = self._get_datetime()

        # 入出力のパス
        self.in_csv_path = None
        self.in_pic_path = None
        self.out_csv_path = None
        self.out_dir_path = None
        # ログ表示ON/OFF
        self.pair_log = False
        # 動画用画像ON/OFF
        self.mov_flag = False
        # マッチングの接続範囲(距離)
        self.matching_range = 10
        # 経路の使用範囲
        self.route_threshold = 10
        # 1ピクセルあたりの実際の距離
        self.pix2m = 6 / 80
        # グラフの描画範囲(±[μm])
        self.graph_range = 3

    def set_in_path(self, in_csv='./', in_pic='./'):
        self.in_csv_path = in_csv
        self.in_pic_path = in_pic

    def set_out_path(self, out_csv='./', out_dir='./', *, out_sub='subdir'):
        if out_sub is not 'subdir':
            self.out_path = out_dir + out_sub + '/'
        else:
            self.out_path = out_dir + '/'

    def log_print(self):
        self.pair_log = True

    def movie_out(self):
        self.mov_flag = True

    def set_range(self, matching_range=10):
        self.matching_range = matching_range

    def set_threshold(self, threshold=10):
        self.route_threshold = threshold

    def set_pix2m(self, pix2m=0.0):
        self.pix2m = pix2m

    def set_graph_range(self, graph_range=0.0):
        self.graph_range = graph_range

    def set_datetime(self, dt):
        self.dt = dt

    def _open_csv(self, path):
        # findmaximum.pyで作成されたCSVファイルを読み込んで表のヘッダ部を除いてリスト化する
        data = []
        with open(path, 'r') as f:
            csv_data = csv.reader(f)
            header = next(csv_data)
            # 以下，行番号の有無に対応
            if header[0] == ' ':
                for row in csv_data:
                    data.append(row[1:])
            else:
                for row in csv_data:
                    data.append(row)
        return data

    def _get_datetime(self):
        return get_datetime()

    def _make_pair_csv(self, path, data):
        # リストからマッチングペアのCSVファイルを作成する．
        with open(path, 'w') as f:
            writer = csv.writer(f, lineterminator='\n')
            writer.writerow(['current', 'next'])
            writer.writerows(data)

    def _make_traj_csv(self, trajs):
        # リストから軌跡のCSVファイルを作成する．
        # 出力先作成
        path = self.out_path + 'traj_csv/'
        print('> Check output dirctory: \"' + path + '\"')
        if os.path.isdir(path) is False:
            print('> Output directory are not found.')
            os.makedirs(path, exist_ok=True)
            print('> New directory created!')
        else:
            print('> Already exist.')

        for i, traj in enumerate(trajs):
            f_path = path + 'route_{:0=5}.csv'.format(i)
            with open(f_path, 'w') as f:
                writer = csv.writer(f, lineterminator='\n')
                writer.writerow(['x', 'y'])
                writer.writerows(traj)

    def _data_load(self):
        # データの読み込み
        print('\nData loading now...')
        ID_counter = 0
        frames = []
        input_file_list = sorted(os.listdir(self.in_csv_path))
        for i, input_fname in enumerate(input_file_list):
            point_data = self._open_csv(self.in_csv_path + '/' + input_fname)
            frame = []
            for point in point_data:
                try:
                    x, y = point
                except:
                    print(i)
                    print(point)
                    exit()
                node = Node(ID_counter, i+1, int(x), int(y))
                ID_counter += 1
                frame.append(node)
            frames.append(frame)
        print('> ' + str(len(frames)) + ' frames loaded.')
        print('> Done.')

        return frames

    def _matching(self, frames):
        # マッチング
        nodes = frames[0]
        for i, next_nodes in enumerate(frames[1:]):
            # ランキング
            for idx, node in enumerate(nodes):
                node.make_rank(next_nodes, neighbor_range=self.matching_range)

            for idx, next_node in enumerate(next_nodes):
                next_node.make_rank(nodes)

            stableMatching = GaleShapley(nodes, next_nodes)
            stableMatching.matching()
            match_list = stableMatching.get_matching_list()
            if self.pair_log is True:
                print('frame [' + str(i) + '] <:::> [' + str(i+1) + ']')
                for pair in match_list:
                    c, n = pair
                    print('pair: [' + str(c) + '] <---> [' + str(n) + ']')
                print('+++++')

            # ペアの書き出し
            # make_pair_csv(out_csv_path % i, match_list)

            # 親子関係の設定
            for node in nodes:
                node.child_ID = node.keeping_node_ID
                for next_node in next_nodes:
                    if next_node.ID is node.keeping_node_ID:
                        next_node.parent_ID = node.ID

            # フレームの保存とリセット
            nodes = next_nodes
            for node in nodes:
                node.reset()

        return frames

    def _tracking(self, frames):
        # トラッキング
        print('\nTracking ...')
        route_list = []
        for t, nodes in enumerate(frames):
            for node in nodes:
                # 始点のノードかつ次フレームへの接続を持つ場合ルートを作成する
                if node.parent_ID is None and node.child_ID is not None:
                    route = Route(t, node.ID)
                    child_ID = node.child_ID

                    # 次フレームへの接続が続く限りルートに記録し続ける
                    while child_ID is not None:
                        route.node_append(child_ID)
                        parent_ID = child_ID
                        for n_nodes in frames:
                            for n_node in n_nodes:
                                if n_node.ID is parent_ID:
                                    child_ID = n_node.child_ID

                    route_list.append(route)
        print('> Done.')

        return route_list

    def _output(self, frames, route_list):
        # 描画
        graph = OverlapGraph(frames, route_list)
        graph.setMaxRange(self.graph_range)
        graph.route_extractor(self.route_threshold)
        moved_point_list, real_scale_list = graph.figout(self.out_path, pix2m=self.pix2m)
        graph.trajout(self.out_path, pix2m=self.pix2m)
        if self.mov_flag is True:
            graph.movout(self.out_path, self.in_pic_path)     # 時間かかる

        return moved_point_list, real_scale_list

    def tracking(self):
        # メインフロー

        # データの読み込み
        frames = self._data_load()

        # マッチング
        frames = self._matching(frames)

        # トラッキング
        route_list = self._tracking(frames)

        # 描画
        moved_point_list, real_scale_list = self._output(frames, route_list)

        # csv出力
        self._make_traj_csv(real_scale_list)

        return frames, route_list, moved_point_list, real_scale_list


if __name__ == '__main__':
    # dir path for csv files
    c_path = './intermediate/maxmumCSVs/first_meeting/k=5_100/'
    p_path = './data/first_meeting/png/enumurate/%05d.png'
    # out_csv_path = '../../output/matching/csv/k=5_80/%05d.csv'
    o_path = './tracking_result/route_view/first_meeting/'

    obj = Tracking()
    obj.set_path(in_csv=c_path, in_pic=p_path, out_csv=None, out_dir=o_path)
    # ログ表示ON/OFF
    obj.set_log_print(True)
    # マッチングの接続範囲(距離)
    obj.set_range(matching_range=10)
    # 経路の使用範囲
    obj.set_threshold(threshold=10)
    # 1ピクセルあたりの実際の距離
    pix2m = 6 / 80
    obj.set_pix2m(pix2m=pix2m)
    # グラフの描画範囲(±[μm])
    obj.set_graph_range(graph_range=3)

    obj.tracking()
