# -*- coding: utf-8 -*-
"""
Created on Thu May  9 22:52:24 2024

@author: Administrator
"""

import cv2 as cv 
import os
from DB.utils import image2json,Imgobj2DB,Imginfo2DB,ImgFeatureMatching
from concurrent.futures import ThreadPoolExecutor
processed_files = set()  
imgset={}
relation={}
def process_file(i,file_names,relation):
    global processed_files
    global imgset
    if i in processed_files:  
        return  # 如果文件已经被处理过，则直接返回  
    processed_files.add(i)  # 标记文件为已处理
    if i not in imgset:
        imgset[i]=cv.imread(i)
    i_img = imgset[i]
    for j in file_names:
        if j == i or j in processed_files:
            continue
        if j not in imgset:
            imgset[j]=cv.imread(j)
        j_img = imgset[j]
        if ImgFeatureMatching(i_img,j_img):  
                if i in relation:  
                    relation[i].append(j)
                else:  
                    relation[i] = [j]  
                processed_files.add(j)
    return relation 
if __name__ == '__main__':
    directory_path=r"E:\WorkWork\bugproject\data_images"
    file_names = []  
    for file_name in os.listdir(directory_path):  
    # 如果需要，你可以添加额外的检查来只包括文件或只包括子目录  
    # 例如，使用os.path.isfile()来检查是否为文件  
        file_path = os.path.join(directory_path, file_name)  
        if os.path.isfile(file_path):
            file_names.append(file_path)  
    futures = []
    pool=ThreadPoolExecutor(max_workers=11)
    for q, file_name in enumerate(file_names):  # 使用 enumerate 获取索引和文件名  
        future = pool.submit(process_file, i=file_name, file_names=file_names, relation=relation)  
        futures.append(future)  
    res = [future.result() for future in futures]  