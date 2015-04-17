function reconImg = C420(imgIn)
	% input:
	%    imgIn: w*h*3 rgb image, w and h must be a multiply of 2
	% output:
	%    reconImg: w*h*3 image, reconstructed
	imgDim = size(imgIn);
	h = imgDim(1);
	w = imgDim(2);
	assert(bitand(w,1) == 0);
	assert(bitand(h,1) == 0);
	assert(imgDim(3) == 3);

	% RGB to YCbCr
	matRGB2YCbCr = [[0.299 0.587 0.114];[-0.169 -0.331 0.500];[0.500 -0.419 -0.081]];
	imgYCbCr = image_color_transform(matRGB2YCbCr, imgIn, w, h);

	% downsample and upsample
	imgYCbCr(:,:,2) = imresize(imresize(imgYCbCr(:,:,2), 0.5), 2);
	imgYCbCr(:,:,3) = imresize(imresize(imgYCbCr(:,:,3), 0.5), 2);

	% YCbCr to RGB
	matYCbCr2RGB = [[1.000 -0.009 1.402];[1.000 -0.344 -0.714];[1.000 1.772 0.010]];
	reconImg = image_color_transform(matYCbCr2RGB, imgYCbCr, w, h);


function T = image_color_transform(colorTransMat, iImg, w, h)
	% A small routine
	tmp = colorTransMat * reshape(permute(iImg, [3 1 2]), [3 (w*h)]);
	T  = tmp(1,:);  T = reshape( T, [h w]);
	C2 = tmp(2,:); C2 = reshape(C2, [h w]);
	C3 = tmp(3,:); C3 = reshape(C3, [h w]);
	T(:,:,2) = C2;
	T(:,:,3) = C3;
