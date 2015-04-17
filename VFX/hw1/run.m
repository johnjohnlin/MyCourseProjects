%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% FILE: run.m
% USAGE: Morph two images.
% AUTHOR: Cobra-Kao, Johnjohnlin
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
close all;
clear all;
clc;

path = set_path();

[img1 img2] = read_image(path);

[start_1 vector_1 start_2 vector_2] = draw_line_pair(img1, img2);

start_both = (start_1 + start_2) ./ 2;
vector_both = (vector_1 + vector_2) ./ 2;

warp_img1 = warp(img1, start_1, vector_1, start_both, vector_both);
warp_img2 = warp(img2, start_2, vector_2, start_both, vector_both);
