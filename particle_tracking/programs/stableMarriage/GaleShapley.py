class GaleShapley():
    # 安定結婚問題のGale-Shapleyのアルゴリズムの実装

    def __init__(self, man_nodes, woman_nodes):
        # コンストラクタ

        # args:
        # man_nodes : 男側のノード
        # woman_nodes : 女側のノード
        self.man_nodes = []
        self.woman_nodes = []

        for i in range(len(man_nodes)):
            if len(man_nodes[i].node_rank):
                self.man_nodes.append(man_nodes[i])
        for i in range(len(woman_nodes)):
            if len(woman_nodes[i].node_rank):
                self.woman_nodes.append(woman_nodes[i])

        # print('---')
        # print(len(self.man_nodes))
        # print('')
        # print(len(self.woman_nodes))

    def matching(self):
        # Gale-Shapleyのアルゴリズムに従ってマッチングを行う

        while self.stable_check() is False:
            for man_node in self.man_nodes:
                # print('')
                # print('node[' + str(man_node.ID) + ']')

                for target_idx, target_ID in enumerate(man_node.node_rank):
                    # if target_ID is None:
                    # print(target_ID)
                    # exit(1)
                    if man_node.kept_status is False:
                        # man_nodeがキープされていない場合
                        # print(target_ID)
                        if target_ID not in man_node.get_dump_list():
                            # man_nodeがすでに振られていない相手の場合
                            for woman_node in self.woman_nodes:
                                if woman_node.ID is target_ID:
                                    judge_status, dump_man = woman_node.judge_keeping(man_node.ID)
                                    man_node.kept_status = judge_status

                                    if judge_status is False:
                                        # 振られた場合
                                        man_node.dump_append(target_ID)

                                    elif judge_status is True:
                                        # キープされた場合
                                        man_node.keeping_node_ID = target_ID
                                        # print('pair: [' + str(man_node.ID) + '] <---> [' + str(target_ID) + ']')

                                        if dump_man is not None:
                                            # 相手が今までキープしていたノードを振る
                                            for dump_node in self.man_nodes:
                                                if dump_node.ID is dump_man:
                                                    dump_node.kept_status = False
                                                    dump_node.dump_append(target_ID)
                                                    dump_node.keeping_node_ID = None
                        
                        # else:
                        #     print('not attack')

    def stable_check(self):
        # 男側の全ノードがキープされていたらTrueを返す
        # また，女性側が埋まっていたらTrueを返す

        for man_node in self.man_nodes:
            if man_node.kept_status is False:
                if man_node.dumped_ID_len == len(man_node.node_rank):
                    return True

                else:
                    return False

        return True

    def get_matching_list(self):
        # 現状のノード to ノードのマッチングを取得して返す

        match_list = []

        for man_node in self.man_nodes:
            match_list.append([man_node.ID, man_node.keeping_node_ID])

        # if len(self.man_nodes) >= len(self.woman_nodes):
        #     for man_node in self.man_nodes:
        #         match_list.append([man_node.ID, man_node.keeping_node_ID])
        # else:
        #     for woman_node in self.woman_nodes:
        #         match_list.append([woman_node.keeping_node_ID, woman_node.ID])

        return match_list


if __name__ == '__main__':
    # テストコード

    import Node as nd
    import numpy as np
    # from time import sleep

    current_nodes = []
    next_nodes = []
    third_nodes = []
    ID_counter = 0

    cX = np.random.rand(30) * 20
    cY = np.random.rand(30) * 10
    for x, y in zip(cX, cY):
        current_nodes.append(nd.Node(ID_counter, 1, x, y))
        print([ID_counter, x, y])
        ID_counter += 1

    print('')
    nX = np.random.rand(30) * 10
    nY = np.random.rand(30) * 10
    for x, y in zip(nX, nY):
        next_nodes.append(nd.Node(ID_counter, 2, x, y))
        print([ID_counter, x, y])
        ID_counter += 1

    print('')
    tX = np.random.rand(30) * 20
    tY = np.random.rand(30) * 10
    for x, y in zip(tX, tY):
        third_nodes.append(nd.Node(ID_counter, 3, x, y))
        print([ID_counter, x, y])
        ID_counter += 1

    print('---')

    # ranking
    for idx, node in enumerate(current_nodes):
        node.make_rank(next_nodes, neighbor_range=None)
        print(node.node_rank)

    print('')
    for idx, next_node in enumerate(next_nodes):
        next_node.make_rank(current_nodes)
        print(next_node.node_rank)

    # exit(1)

    stableMat = GaleShapley(current_nodes, next_nodes)
    stableMat.matching()
    match_list = stableMat.get_matching_list()

    for pair in match_list:
        c, n = pair
        print('pair: [' + str(c) + '] <---> [' + str(n) + ']')

    for node in next_nodes:
            node.reset()

    # ranking
    for idx, node in enumerate(next_nodes):
        node.make_rank(third_nodes, neighbor_range=5)
        print(node.node_rank)

    print('')
    for idx, node in enumerate(third_nodes):
        node.make_rank(next_nodes)
        print(node.node_rank)

    stableMat = GaleShapley(next_nodes, third_nodes)
    stableMat.matching()
    match_list = stableMat.get_matching_list()

    for pair in match_list:
        c, n = pair
        print('pair: [' + str(c) + '] <---> [' + str(n) + ']')
