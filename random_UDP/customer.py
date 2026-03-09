import socket
import json
import time

# 目标服务器地址和端口（必须与 QT 软件中监听的端口一致）
SERVER_IP = "127.0.0.1"
SERVER_PORT = 8888


def fetch_sensor_data():
    # 1. 创建 UDP 套接字 (IPv4, Datagram)
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    # 2. 设置超时时间，避免 QT 服务端没开时一直卡住等待
    client_socket.settimeout(2.0)

    try:
        # 3. 向服务器发送触发请求。
        # QT端只要收到任何UDP数据报就会生成数据，所以这里发送内容随意，比如 "GET"
        message = b"GET"
        client_socket.sendto(message, (SERVER_IP, SERVER_PORT))
        print(f"--> 已向 {SERVER_IP}:{SERVER_PORT} 发送请求...")

        # 4. 接收服务器返回的数据 (最大接收1024字节)
        data, server_address = client_socket.recvfrom(1024)

        # 5. 解码并解析 JSON
        json_string = data.decode('utf-8')
        sensor_data = json.loads(json_string)

        print(f"<-- 收到数据:")
        print(f"    [温度] {sensor_data.get('temperature')} ℃")
        print(f"    [湿度] {sensor_data.get('humidity')} %")
        print(f"    [PM2.5] {sensor_data.get('pm25')} ug/m³")
        print(f"    [CO2] {sensor_data.get('co2')} ppm\n")

    except socket.timeout:
        print("❌ 请求超时：未收到响应，请检查 QT 模拟器是否已点击“启动服务”。\n")
    except json.JSONDecodeError:
        print(f"❌ 解析错误：收到的数据不是标准 JSON 格式。原始数据: {data}\n")
    except Exception as e:
        print(f"❌ 发生未知错误: {e}\n")
    finally:
        # 6. 关闭套接字
        client_socket.close()


if __name__ == "__main__":
    print("=== 环境数据获取客户端已启动 ===")

    # 模拟连续请求 3 次，每次间隔 1 秒
    for i in range(1, 4):
        print(f"--- 第 {i} 次请求 ---")
        fetch_sensor_data()
        time.sleep(1)