function dst = warp(src, srcStarts, srcVectors, dstStarts, dstVectors)
	% input:
		% src
			% input images
			% (img height)*(img width)*3
			% note: same format as imread
		% srcStarts
			% start points of lines of src images
			% size is (#lines)*2
			% each row is x, y
		% srcVectors
			% vectors of lines of src images
			% namely "end point - start point"
			% size is (#lines)*2
			% each row is dx, dy
		% dstStarts
		% dstVectors
			% the counterpart of srcStarts and srcVectors in destination image
			% size are both (#lines)*2
	% output:
		% dst
			% morphed image
	% misc:
		% (x, y) is distance from left and top, as normal convention
		% namely, to access a pixel (x, y) , use src(y, x)
	w = size(src, 2);
	h = size(src, 1);
	const_a = (w + h) / 30;
	numLines = size(srcVectors, 1);
	p = zeros(2, 1);
	oo = ones(numLines, 1, 'uint32');
	srcStarts_cell  = mat2cell(srcStarts' , 2, oo);
	srcVectors_cell = mat2cell(srcVectors', 2, oo);
	dstStarts_cell  = mat2cell(dstStarts' , 2, oo);
	dstVectors_cell = mat2cell(dstVectors', 2, oo);
	function w = weight_function(s, t)
		% calculating weight of s and line(s, s+t)
		% both are 2*1 matrix
		% first calculating the distance
		x = p-s;
		t_leng = sqrt(t' * t);
		t_v = [-t(2); t(1)]; % t's vertical vector
		d = abs(t_v' * x) / t_leng;
		% the weight function
		w = (t_leng) / (const_a + d);
	end
	function y = proj(q, r, s, t)
		% calculating corresponding point using 1 line pairs
		% p is the point coordinate in dst image
		% line(q, q+r) is in src image, line(s, s+t) is in dst image
		% both are 2*1 matrix
		A = [t [t(2); -t(1)]];
		B = [r [r(2); -r(1)]];
		transMat = B * inv(A);
		y = transMat*(p-s) + q;
	end
	function [retx rety] = weighted_proj(px, py)
		p = [px; py];
		xs = cellfun(@proj,...
			srcStarts_cell, srcVectors_cell,...
			dstStarts_cell, dstVectors_cell, 'UniformOutput', 0);
		ws = cellfun(...
			@weight_function,...
			dstStarts_cell, dstVectors_cell, 'UniformOutput', 0);
		% weighted x
		wxs = cellfun(@times, ws, xs, 'UniformOutput', 0);
		avgX = sum([wxs{:}], 2)/sum([ws{:}]);
		retx = int32(max(min(avgX(1) + 0.5, h), 1));
		rety = int32(max(min(avgX(2) + 0.5, w), 1));
	end

	disp 'Generating index matrix...'
	tic;
	[xx yy] = meshgrid(1:w, 1:h);
	[indX indY] = arrayfun(@weighted_proj, xx, yy);
	toc;

	disp 'Converting index...'
	tic;
	dst_cell = arrayfun(@(y, x) src(y, x, :), indY, indX, 'UniformOutput', 0);
	toc;

	disp 'cell2mat...'
	tic;
	dst = cell2mat(dst_cell);
	toc;
end
