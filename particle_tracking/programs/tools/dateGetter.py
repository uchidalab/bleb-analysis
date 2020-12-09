from datetime import datetime, timedelta, timezone


def get_datetime():
    # タイムゾーン生成
    JST = timezone(timedelta(hours=+9), 'JST')
    # 日時の取得
    dt = datetime.now(JST).strftime('%Y%m%d_%H%M%S_%a')
    return dt
