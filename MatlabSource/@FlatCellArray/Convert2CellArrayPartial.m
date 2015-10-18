function CellArray = Convert2CellArrayPartial(obj, DepthStartInd, BegInds, EndInds)
%CONVERT2CELLARRAYPARTIAL This function converts the partial Flat Cell Array to Cell Array
%
%    CellArray = Convert2CellArrayPartial(obj, DepthStartInd, BegInds, EndInds)
%   
%  INPUT ARGUMENTS -
%  
%   * obj           -    'obj' is the object representing the full Flat Cell 
%                        array of which a part is chosen by the following 
%                        input arguments to convert into a cell.
%   
%   * DepthStartInd -    This is the scalar that lies between 1 and depth of 
%                        'obj' that gives us the depth from which we will 
%                        start considering the Flattened Cell Array to be 
%                        converted
%   
%   * BegInds, EndInds - 
%   
%              These are vectors of size 
%                  
%                  'Depth of obj' - DepthStartInd + 1
%              
%              Such that 
%              
%                  obj.PartitionIndex{DepthStartInd + j - 1}(BegInds(j)+1:EndInds+1) 
%              
%              Note that the above range contains the element beyond the
%              end and the +1 is because BegInds and EndInds are given with
%              the first element indexed by 0.
%              
%              is the part of obj.PartitionIndex for each j in 
%              (1:'Depth of obj' - DepthStartInd + 1) which is considered
%              to be a part of the partial Flat Cell Array which is to be 
%              converted

	FullDepth = length(obj.PartitionIndex);
	ActualDepth = FullDepth - DepthStartInd + 1;

	% recursively convert into cell array
	if ActualDepth == 1
		NElemsAtCurrLevel = EndInds(1) - BegInds(1);
		CellArray = cell(NElemsAtCurrLevel, 1);
		CellArray(:) = arrayfun(@(Beg,End) obj.Data(Beg+1:End), ...
							obj.PartitionIndex{DepthStartInd}(BegInds+1:EndInds), obj.PartitionIndex{DepthStartInd}(BegInds+2:EndInds+1),...
							'UniformOutput', false);
	else
		NElemsAtCurrLevel = EndInds(1) - BegInds(1);
		CellArray = cell(NElemsAtCurrLevel, 1);

		SubBegInds = BegInds(2:end);
		SubEndInds = zeros(size(EndInds(2:end)));

		for i = 1:NElemsAtCurrLevel
			% Calculating SubEndInds
			SubEndInds(1) = obj.PartitionIndex{DepthStartInd}(BegInds(1) + i + 1);
			for j = DepthStartInd+2:FullDepth
				SubEndInds(j - DepthStartInd) = obj.PartitionIndex{j-1}(SubEndInds(j-1 - DepthStartInd) + 1);
			end

			% Calculating and assigning sub cell array to CellArray
			CellArray{i} = obj.Convert2CellArrayPartial(DepthStartInd+1, SubBegInds, SubEndInds);

			% Swapping SubEndInds and SubBegInds
			SubBegInds = SubEndInds;
		end
	end

end

