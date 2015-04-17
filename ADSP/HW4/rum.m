img1 = double(rgb2gray(imread('lena.bmp'))) * (1/255.0);
% img2 = img1;
img2 = img1 * 0.1;
img3 = zeros(512);

SSIM(img1, img1, 1E-4, 9E-4)
SSIM(img1, img2, 1E-4, 9E-4)
SSIM(img1, img3, 1E-4, 9E-4)
