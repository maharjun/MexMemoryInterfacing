function filter(FlatCellArr, FiltIndexVect, Depth)
%FILTER This vector filters the level Level to contain the elements indexed
%in FiltIndexVect. 
% 
% Syntax:
% 
%   filter(FlatCellArr, FiltIndexVect, Depth)
% 
% Notes:
% 
%   if Depth <= FlatCellArr.Depth then 
%       PartitionLevel{i} is filtered accordingly
%   elseif Depth is unspecified or Depth == FlatCellArr.Depth + 1
%       Data is filtered accordingly

% Default Assignment
if nargin == 2
    Depth = FlatCellArr.Depth + 1;
end

% Validating Depth
if Depth <= 0 || Depth > FlatCellArr.Depth + 1
    ME = MException('FlatCellArray:InvalidInput', 'The Depth (%d) must lie between 1 and The Depth of the FLatCellArray (%d)', Depth, FlatCellArr.Depth);
    throw(ME);
end

% Recursion Initialization Step
if islogical(FiltIndexVect)
    FiltIndexVect = find(FiltIndexVect);
else
    FiltIndexVect = unique(FiltIndexVect);
end

if Depth <= FlatCellArr.Depth
   % Find Filtering Indices for sub-level 
   BegIndexVect = FlatCellArr.PartitionIndex{Depth}(FiltIndexVect);
   EndIndexVect = FlatCellArr.PartitionIndex{Depth}(FiltIndexVect+1);
   SubLevelInds = cell(length(BegIndexVect), 1);
   for i = 1:length(BegIndexVect)
       SubLevelInds{i} = transpose(BegIndexVect(i)+1:EndIndexVect(i));
   end
   SubLevelInds = cell2mat(SubLevelInds);
   
   % Filter SubLevel
   filter(FlatCellArr, SubLevelInds, Depth+1);
end

if Depth > 1
   % Edit Parent to be consistent with new PartitionIndex
   
   % Calculate Vector of Corresponding Parent PartitionIndex
   % i.e. PartitionIndex{Depth}(i) belongs to PartitionIndex{Depth-1}(CorrParentPartIndex(i))
   CorrParentPartIndex = accumarray(...
                            FlatCellArr.PartitionIndex{Depth-1}(1:end)+1, ...
                            ones(length(FlatCellArr.PartitionIndex{Depth-1}), 1), ...
                            [FlatCellArr.PartitionIndex{Depth-1}(end)+1, 1]);
   CorrParentPartIndex = cumsum(CorrParentPartIndex(1:end-1));
   
   % Filter the corresponding parent index vect
   FiltCorrParentPartIndex = CorrParentPartIndex(FiltIndexVect);
   
   % count the number of elements corresponding to each parent
   if ~isempty(FiltIndexVect)
       ChildrenCount = accumarray(FiltCorrParentPartIndex, ...
                                  ones(length(FiltIndexVect),1), ...
                                  [length(FlatCellArr.PartitionIndex{Depth-1})-1, 1]);
   else
       ChildrenCount = zeros(length(FlatCellArr.PartitionIndex{Depth-1})-1, 1);
   end
   % calculate new parent vector y cumsumming no. of children
   FlatCellArr.PartitionIndex{Depth-1} = uint32([0; cumsum(ChildrenCount)]);
end

% Filter relevant vector
if Depth <= FlatCellArr.Depth
    FlatCellArr.PartitionIndex{Depth} = FlatCellArr.PartitionIndex{Depth}(FiltIndexVect);
else
    FlatCellArr.Data = FlatCellArr.Data(FiltIndexVect);
end

end

