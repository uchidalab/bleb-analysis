
import cv2
import numpy as np
import os
import sys

# local modules
sys.path.append('./programs/')
import tools.mystd as std


def LOGfilter(src_img, k_size, sigma):
    gaussian_img = cv2.GaussianBlur(src_img, (k_size, k_size), sigma)
    log_img = cv2.Laplacian(gaussian_img, ddepth=cv2.CV_32F, ksize=k_size)
    log_img[log_img > 0] = 0
    log_img = log_img * -1
    log_img = abs(log_img)
    log_min = np.min(log_img)
    log_max = np.max(log_img)
    log_nlz = (log_img - log_min) / (log_max - log_min) * 255
    log_nlz = log_nlz.astype(np.uint8)
    return log_nlz


def main():
    cat_list = std.get_dir_list(super_path)
    for cat_name in cat_list:
        cat_path = super_path + cat_name + '/'
        subdir_list = std.get_dir_list(cat_path)
        for d_i, dir_name in enumerate(subdir_list):
            target_path = cat_path + dir_name + '/'
            output_path = out_path + cat_name + '/{:02d}/'.format(d_i+1)
            std.make_dir(output_path)
            file_list = std.get_file_list(target_path, input_file_extension)
            file_path_list = [target_path + file_name for file_name in file_list]

            images = std.image_load(file_path_list, 'gray')
            for i, image in enumerate(images):
                log_img = LOGfilter(image, 5, 0)
                cv2.imwrite(output_path + '/%05d.' % i + output_file_extention, log_img)


if __name__ == '__main__':
    # options
    super_path = './input/'
    out_path = './output/LoG/'
    input_file_extension = 'png'
    output_file_extention = 'png'

    main()
