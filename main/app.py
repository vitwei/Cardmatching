# -*- coding: utf-8 -*-
"""
Created on Sun May 19 16:36:01 2024

@author: Administrator
"""

from flask import *
import socket
import json
import os
from DB.utils import requestbaseimgpool,custom_json_serializer
SERVER_IP = '127.0.0.1'
SERVER_PORT = 50300
app = Flask(__name__)
IMAGE_FOLDER = os.path.join(os.getcwd(), 'images')

@app.route('/')
def index():
    # 在路由处理函数中访问请求对象
    client_ip = request.remote_addr
    return f'Client IP Address: {client_ip}'

@app.route('/images/<filename>', methods=['GET'])
def get_image(filename):
    # 构造图片文件的完整路径
    image_path = os.path.join(IMAGE_FOLDER, filename)
    print(image_path)
    # 检查文件是否存在
    if os.path.exists(image_path):
        return send_file(image_path)
    else:
        # 如果文件不存在，返回404错误
        return abort(404, description="Resource not found")

@app.route('/hello', methods=['GET'])
def hello():
    return jsonify(message="Hello, World!")

@app.route('/echo', methods=['POST'])
def echo():
    data = request.get_json()
    return jsonify(data)

@app.route('/cardmatch', methods=['POST'])
def cardmatch():
    data = request.get_json()
    if not data:
        return jsonify({"error": "No input data provided"}), 400
    carddesjson= json.dumps(data)
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        client_socket.connect((SERVER_IP, SERVER_PORT))
        client_socket.sendall(carddesjson.encode())
        print("Message sent successfully.")
        response = b''  # 初始化一个空字节串
        data = client_socket.recv(1024)
        response += data  # 将接收到的数据添加到响应中
        print("Received response:", response.decode())
        if response.decode()=='False':
            return jsonify(message="False"),200
        resjson=response.decode()
        dbdata=requestbaseimgpool(resjson)
        print(dbdata)
        img=[]
        price=[]
        bidcount=[]
        time=[]
        for i in dbdata:
            img.append(i[0])
            price.append(i[2])
            bidcount.append(i[3])
            time.append(i[4])     
        respoense= {
            "message": "ok",
            "img":img,
            "price": json.dumps(price),
            "bidcount": json.dumps(bidcount),
            "time": json.dumps(time,default=custom_json_serializer)
            }
        return jsonify(respoense), 200
    except Exception as e:
        print("Error:", e)
        return jsonify(error="An error occurred"), 500
    finally:
    # 关闭套接字
        client_socket.close()

if __name__ == '__main__':
    app.run(debug=True, port=50301)
