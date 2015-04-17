A = im2double(imread('test.png'));
B = C420(A);
imwrite(B, 'test_reconstruct.png');
