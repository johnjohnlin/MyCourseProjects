function [start_1 vector_1 start_2 vector_2] = draw_line_pair(img1, img2)
% DRAW_LINE_PAIR  A gui to let user specify line pairs to align.
%    DRAW_LINE_PAIR(IMG1, IMG2) returns an array that contains the start point
%    and vector of the corresponding line pairs of both image.
%
%    [start_1 vector_1 start_2 vector_2] = draw_line_pair(img1, img2)
%    where 'start' is the start point the line, and vector is vector of it.
%    Each has a size of (#lines)*2, each row is (x, y)

  n_line = 0; % number of line pairs
  start_1 = []; % for img1
  vector_1 = [];
  start_2 = []; % for img2
  vector_2 = [];

  fig = figure();
  while true
    % A dialog to select action
    choice = questdlg(...
        ['What now? (' num2str(n_line) ' line pair(s) now)'],... % message
        'menu',... % title bar
        'Add a new line pair',... % option
        'Done',... % option
        'Add a new line pair'); % default option
    % Add a line pair or quit
    switch choice
      case 'Add a new line pair'
        n_line = n_line + 1;
        dim_ratio = 0.5;
        % for image 1
        subplot(1, 2, 1);
        imshow(uint8(img1), 'Border', 'tight');
        subplot(1, 2, 2);
        imshow(uint8(img2 * dim_ratio), 'Border', 'tight');
        subplot(1, 2, 1);
        title('Click on start point of the line pair.');
        [y1 x1] = ginput(1);
        title('Click on end point of the line pair.');
        [y2 x2] = ginput(1);
        start_1 = [start_1; [y1 x1]];
        vector_1 = [vector_1; [(y2 - y1) (x2 - x1)]];
        title('');
        % for image 2
        subplot(1, 2, 1);
        imshow(uint8(img1 * dim_ratio), 'Border', 'tight');
        subplot(1, 2, 2);
        imshow(uint8(img2), 'Border', 'tight');
        subplot(1, 2, 2);
        title('Click on start point of the line pair.');
        [y1 x1] = ginput(1);
        title('Click on end point of the line pair.');
        [y2 x2] = ginput(1);
        start_2 = [start_2; [y1 x1]];
        vector_2 = [vector_2; [(y2 - y1) (x2 - x1)]];
        title('');
      case 'Done'
        break
    end
  end
  close(fig);
end
