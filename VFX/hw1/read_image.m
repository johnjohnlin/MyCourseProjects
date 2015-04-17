function [img1 img2] = read_image(path)
% READ_IMAGE() reads two images from the specified path
%    [img1 img2] = read_image(path);

  img1 = double(imread(path.img1));
  img2 = double(imread(path.img2));

end
