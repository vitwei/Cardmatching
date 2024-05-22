# -*- coding: utf-8 -*-
"""
Created on Wed May  8 12:37:37 2024

@author: Administrator
"""

from multiprocessing import Pool
import os
from DB.utils import image2json,Imgobj2DB
if __name__ == '__main__':
    directory_path=r"E:\WorkWork\bugproject\data_images"
    file_names = []  
    for file_name in os.listdir(directory_path):  
    # 如果需要，你可以添加额外的检查来只包括文件或只包括子目录  
    # 例如，使用os.path.isfile()来检查是否为文件  
        file_path = os.path.join(directory_path, file_name)  
        if os.path.isfile(file_path):
            file_names.append(file_path)  
    with Pool(processes=10) as pool:  
        imglist = pool.map(image2json,file_names)  
    with Pool(processes=10) as pool:  
        results = pool.map(Imgobj2DB,imglist)

        
        