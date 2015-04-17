function draw_hw(fileName)
	fp = fopen(fileName);
	s = fscanf(fp, '%d%d', 2);
	w = s(1);
	h = s(2);
	mvs = fscanf(fp, '%f%f', [2 (w*h)]);
	fclose(fp);
	u = reshape(mvs(1,:), [w h]).';
	v = reshape(mvs(2,:), [w h]).';
	u = imresize(u, 0.05);
	v = imresize(v, 0.05);
	[hs,ws] = size(u);
	quiver(1:ws, hs:-1:1, u, -v, 3);
	axis([0 ws 0 hs])
