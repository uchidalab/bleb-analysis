import sys
import traceback
from math import sqrt
import numpy as np


class Node():
    # 安定結婚マッチングに使うノードのクラス

    def __init__(self, id, t, x, y):
        # コンストラクタ

        # args:
        #  ID : ノードのID
        #   t : ノードが属しているTIME
        #   x : ノードのx座標
        #   y : ノードのy座標
        self.ID = id
        self.t = t
        self.x = x
        self.y = y

        # init:
        # for man ===
        #       kept_status : 自分がキープされているか
        #         dumped_ID : ペアになれなかったノードを記録
        #     dumped_ID_len : dumped_IDの長さ
        # for woman ===
        #   keeping_node_ID : 現在ペアとしてキープしているノードのID
        # for both ===
        #         parent_ID : 接続的親のID
        #          child_ID : 接続的子のID
        self.kept_status = False
        self.dumped_ID = []
        self.dumped_ID_len = 0
        self.keeping_node_ID = None
        self.parent_ID = None
        self.child_ID = None

    def reset(self):
        # 初期状態に戻す

        self.kept_status = False
        self.dumped_ID = []
        self.dumped_ID_len = 0
        self.keeping_node_ID = None

        self.distance_list = []
        self.node_rank = []

    def get_dump_list(self):
        # dump_IDを返す
        if type(self.dumped_ID) != list:
            self.dumped_ID = [self.dumped_ID]
        # print('dumps' + str(self.dumped_ID))
        return self.dumped_ID

    def dump_append(self, ID):
        # 振られたノードの番号を追加する
        if type(self.dumped_ID) != list:
            self.dumped_ID = [self.dumped_ID]

        self.dumped_ID.append(ID)
        self.dumped_ID_len = len(self.dumped_ID)
        # print('dump append:' + str(self.dumped_ID))

    def judge_keeping(self, man_ID):
        # 告白してきた相手をキープするかどうかを判断する(woman用)

        status = False
        dump = None

        # まだノードをキープしていない場合
        if self.keeping_node_ID is None:
            for target_node in self.node_rank:
                if target_node == man_ID:
                    self.keeping_node_ID = man_ID
                    status = True

        # すでにノードをキープしている場合
        else:
            for target_node in self.node_rank:
                if target_node == self.keeping_node_ID:
                    break

                elif target_node == man_ID:
                    dump = self.keeping_node_ID
                    self.keeping_node_ID = man_ID
                    status = True

        # キープしたか否かを返却(True/False)
        return status, dump

    def make_rank(self, otherNodes, *, neighbor_range=None, calc_method='L2'):
        # 渡された各他ノードとの距離からマッチング率をランキング化．
        # リストを昇順にソートしたインデックスを保持する．

        self.distance_list = []

        if calc_method == 'L2':
            for otherNode in otherNodes:
                __distance = self.__calc_distanceL2(otherNode)
                self.distance_list.append(__distance)

        elif calc_method == 'L1':
            for otherNode in otherNodes:
                __distance = self.__calc_distanceL1(otherNode)
                self.distance_list.append(__distance)

        else:
            try:
                raise Exception
            except:
                traceback.print_exc()
                print(
                    'AttributeError: \'calc_method\' option has no attribute \' '
                    + calc_method + '\' ')
                print('You should set \'L1\', \'L2\' or nothing.')
                sys.exit(1)
        __distance_list = np.array(self.distance_list)
        sorted_index = np.argsort(__distance_list)

        sorted_ID = []
        if neighbor_range is None:
            for i in sorted_index:
                sorted_ID.append(otherNodes[i].ID)
            self.node_rank = sorted_ID

        else:
            sorted_distance = np.sort(__distance_list)
            effective_index = sorted_index[sorted_distance <= neighbor_range]

            for i in effective_index:
                sorted_ID.append(otherNodes[i].ID)
            self.node_rank = sorted_ID

    def __calc_distanceL2(self, targetNode):
        # 対象ノードとの距離をユークリッド距離で計算する
        return sqrt((self.x - targetNode.x)**2 + (self.y - targetNode.y)**2)

    def __calc_distanceL1(self, targetNode):
        # 対象ノードとの距離をマンハッタン距離で計算する
        return abs(self.x - targetNode.x) + abs(self.y - targetNode.y)


if __name__ == '__main__':
    # テストコード
    testnode1 = Node(0, 1, 13, 15)
    testnode2 = Node(1, 1, 3, 4)
    testnode3 = Node(2, 1, 5, 5)

    testnode4 = Node(3, 2, 1, 2)
    testnode5 = Node(4, 2, 10, 10)

    current_nodes = [testnode1, testnode2, testnode3]
    next_nodes = [testnode4, testnode5]

    # ranking
    for idx, node in enumerate(current_nodes):
        node.make_rank(next_nodes, neighbor_range=10)
        print('Node[' + str(node.ID) + ']')
        print(node.distance_list)
        print(node.node_rank)
    print('---')
    for idx, next_node in enumerate(next_nodes):
        next_node.make_rank(current_nodes, neighbor_range=10)
        print('Node[' + str(next_node.ID) + ']')
        print(next_node.distance_list)
        print(next_node.node_rank)
