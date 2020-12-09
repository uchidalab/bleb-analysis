
class Route():
    # トラッキングの経路情報を格納するクラス

    def __init__(self, t, parent_ID):
        # コンストラクタ

        # args:
        #   t : この経路の開始のTIME
        #   Parent_ID : この経路の始点ノードのID
        self.t = t
        self.route = [parent_ID]

        # init:
        # route_length : この経路の現在の長さ
        self.route_length = None

    def node_append(self, node_ID):
        # 経路にノードを追加して長さを保存しておく
        
        if type(self.route) != list:
            self.route = [self.route]

        self.route.append(node_ID)
        self.route_length = len(self.route)

    def route_print(self):
        # 保持している経路情報を標準出力する

        print('%05d' % self.t, end=' ')
        for i, ID in enumerate(self.route):
            if i is 0:
                print('[' + str(ID) + ']', end=' ')
            
            else:
                print('-->', end=' ')
                print('[' + str(ID) + ']', end=' ')
        print()

