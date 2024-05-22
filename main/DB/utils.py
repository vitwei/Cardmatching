# -*- coding: utf-8 -*-
"""
Created on Wed May  8 13:26:11 2024

@author: Administrator
"""
from datetime import datetime
import random  
import os  
import cv2 as cv 
import json
import psycopg
import pandas as pd
import cv2 as cv
import socket
import json
def image2json(path_to_dir):  
    # 遍历目录中的所有文件  
    
    if os.path.isfile(path_to_dir):  
            # 尝试使用OpenCV读取图片  
        try:  
            img = cv.imread(path_to_dir)  
            if img is not None:  # 确保图片读取成功  
                    # 将图片转换为灰度图  
                gray_img = cv.cvtColor(img, cv.COLOR_BGR2GRAY)  
                sift = cv.SIFT_create()
                kp, des = sift.detectAndCompute(gray_img,None)
                des=des.flatten().tolist()
                json_str = json.dumps(des)
                return [os.path.basename(path_to_dir),json_str]
            else:  
                print(f"Failed to read {path_to_dir}")  
        except Exception as e:  
                # 读取或转换图片时可能会抛出异常（例如，文件不是图片）  
            print(f"Error processing {path_to_dir}: {e}")  

def Imgobj2DB(imgdata):
    
    with psycopg.connect("dbname=Cardtest user=postgres password=123") as conn:
        try:
    
    # Open a cursor to perform database operations
            with conn.cursor() as cur:
        
                cur.execute(
            "INSERT INTO imgobj (img, imgdesjson) VALUES (%s, %s)",
            (imgdata[0], imgdata[1]))
            
        # Query the database and obtain data as Python objects.

                conn.commit()
        except Exception as e:
            print(f"Error processing {imgdata[0]}: {e}")  
            conn.rollback()
            
            
def Imginfo2DB(path_to_dir):
    datacsv=pd.read_excel(path_to_dir)
    with psycopg.connect("dbname=Cardtest user=postgres password=123") as conn:
        for index, row in datacsv.iterrows():
            try:
            
    # Open a cursor to perform database operations
                with conn.cursor() as cur:
        
                    cur.execute(
            "INSERT INTO imginfo (Img, dealtime,textcontent,price,bidcount) VALUES (%s, %s,%s,%s,%s)",
            (row[4], row[0],row[1],row[2],row[3]))
            
        # Query the database and obtain data as Python objects.

                    conn.commit()
            except Exception as e:  
                # 读取或转换图片时可能会抛出异常（例如，文件不是图片）  
                print(f"Error processing {path_to_dir}: {e}")  
                conn.rollback()
                
def requestbaseimgpool(json):
    with psycopg.connect("dbname=Cardtest user=postgres password=123") as conn:
        try:
            with conn.cursor() as cur:
                cur.execute(
            "SELECT img,imgpath,price,bidcount,dealtime FROM public.basecardpool WHERE imgpath=%s",
            (json,))
                conn.commit()
                data = cur.fetchall()
                return data
        except Exception as e:
            conn.rollback()
            
            
def custom_json_serializer(obj):
    if isinstance(obj, datetime):
        return obj.isoformat()
    else:
        raise TypeError("Type not serializable")
            
def ImgFeatureMatching(img1,img2):
    orb = cv.ORB_create()
    # Detect keypoints and compute descriptors on CPU
    kps1, descs1 = orb.detectAndCompute(img1, None)
    kps2, descs2 = orb.detectAndCompute(img2, None)
    # Upload descriptors to GPU memory
    d_descs1 = cv.cuda_GpuMat()
    d_descs1.upload(descs1)
    d_descs2 = cv.cuda_GpuMat()
    d_descs2.upload(descs2)

    # Create CUDA-accelerated BFMatcher with Hamming distance
    matcher_gpu = cv.cuda.DescriptorMatcher_createBFMatcher(cv.NORM_HAMMING)

    # Match descriptors on GPU
    matches = matcher_gpu.knnMatch(d_descs1, d_descs2, k=2)
    good = []
    for m, n in matches:
        if m.distance < 0.75 * n.distance:
            good.append([m])
    if len(good)>=350:
        return True
    else:
        return False


def updatecall():    
    SERVER_IP = '127.0.0.1'
    SERVER_PORT = 50300
    message = 'update'
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        client_socket.connect((SERVER_IP, SERVER_PORT))
                # 发送消息
        client_socket.sendall(message.encode())
        print("Message sent successfully.")

                # 接收来自服务器的响应
        response = b''  # 初始化一个空字节串
        while True:
            data = client_socket.recv(1024)
            if not data:
    # 如果接收到空数据，表示服务器已经关闭连接，停止接收
                break
        response += data  # 将接收到的数据添加到响应中
    
    # 打印接收到的响应
        print("Received response:", response.decode())
    
    except Exception as e:
            print("Error:", e)
    
    finally:
    # 关闭套接字
            client_socket.close()                  
def testcall():    
    img = cv.imread(r"E:\WorkWork\bugproject\test.jpg")  
    if img is not None:  # 确保图片读取成功  
            # 将图片转换为灰度图  
        gray_img = cv.cvtColor(img, cv.COLOR_BGR2GRAY)  
        sift = cv.SIFT_create()
        kp, des = sift.detectAndCompute(gray_img,None)
        des=des.flatten().tolist()
        json_str = json.dumps(des)
        SERVER_IP = '127.0.0.1'
        SERVER_PORT = 50300
    
    # 要发送的消息
        message = json_str
    
    # 创建一个TCP套接字
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
        try:
        # 连接到服务器
            client_socket.connect((SERVER_IP, SERVER_PORT))
        # 发送消息
            client_socket.sendall(message.encode())
            print("Message sent successfully.")
    
        # 接收来自服务器的响应
            response = b''  # 初始化一个空字节串
            data = client_socket.recv(1024)
            response += data  # 将接收到的数据添加到响应中
    
        # 打印接收到的响应
            print("Received response:", response.decode())
    
        except Exception as e:
            print("Error:", e)
    
        finally:
        # 关闭套接字
            client_socket.close()