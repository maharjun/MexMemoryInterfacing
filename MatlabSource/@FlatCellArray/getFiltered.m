function [ FilteredFlatCellArr ] = getFiltered(FlatCellArr, varargin)
%GETFILTERED Returns a filtered copy of the given FCA. see filter for more
%details regarding filtering
%   
% Syntax:
% 
%   FlatCellArr = filter(FlatCellArr, FiltIndexVect, Depth)

FilteredFlatCellArr = FlatCellArr.copy();
FilteredFlatCellArr.filter(varargin{:});
end

