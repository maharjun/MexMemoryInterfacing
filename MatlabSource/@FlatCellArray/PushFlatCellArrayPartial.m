function BegInds = PushFlatCellArrayPartial(FlatCellArr, CellArray, DepthStartInd, BegInds)
%PUSHFLATCELLARRAYPARTIAL Flattens the given Cell Array into specified position
%
%    PushFlatCellArrayPartial(FlatCellArray, CellArray, DepthStartInd, BegInds)
%
%  INPUT ARGUMENTS:
%  
%  * FlatCellArray - 
%  
%      The Flat Cell Array into which the flatened CellArray has to be
%      pushed. This object, being a handle object, is passed by reference
%  
%  * CellArray - 
%  
%      The Cell Array which needs to be flattened and pushed. Presumed to 
%      be valid.
%  
%  * DepthStartIndex - 
%  
%      This is the scalar that lies between 1 and depth of FlatCellArray 
%      that gives us the depth at which the Flattened Cell Array will be 
%      pushed. The depth of the cell array itself will be 
%  
%          CurrCellDepth = Depth(FlatCellArray) - DepthStartIndex + 1
%  
%  * BegInds -
%  
%      BegInds is a vector of length CurrCellDepth such that
%  
%          FlatCellArray.PartitionIndex{DepthStartIndex+i-1}(BegInds(i):..)
%      
%      Is the vector that the ith level of CellArray will be pushed into.
%  
%  OUTPUT ARGUMENTS
%  
%  * BegInds - 
%  
%      Returns the appropriate BegInds after pushing the Flattened Cell 
%      Array
%  
%  WORKING NOTES:
%  
%    This function flattens the given array and DOES NOT push the beyond 
%    the end element as this function is intended to be used as an 
%    intermediate function. The Beyond the end element is inserted by the 
%    parent flattening function.
%    
%    The Value offset for a pushing into a particular level requires knowledge 
%    of the beyond the end element of that level. But BeyondTheEnd(i) can 
%    be inferred from BegInds(i+1).

	FullDepth = length(FlatCellArr.PartitionIndex);
	ActualDepth = FullDepth - DepthStartInd + 1;
	
	% recursively flattining and pushing
	if ActualDepth == 1
		% In case of a cell array of vectors
		NElemsatCurrLevel = length(CellArray);
		
		PartIndInsertIndex = BegInds(1) + 1; % Convert 0-start to 1-start
		PartIndInsertValue = BegInds(2);     % Beyond the end elem for PartitionIndex{FullDepth}
		DataInsertIndex    = BegInds(2) + 1; % Convert 0-start to 1-start
		
		for i = 1:NElemsatCurrLevel
			CurrInsertVector = CellArray{i};
			FlatCellArr.PartitionIndex{FullDepth}(PartIndInsertIndex) = PartIndInsertValue;
			FlatCellArr.Data(DataInsertIndex:DataInsertIndex+length(CurrInsertVector)-1) = CurrInsertVector;
			
			% Updating Variables
			PartIndInsertIndex = PartIndInsertIndex + 1;
			PartIndInsertValue = PartIndInsertValue + length(CurrInsertVector);
			DataInsertIndex    = DataInsertIndex    + length(CurrInsertVector);
		end
		
		% Updating BegInds
		BegInds(1) = PartIndInsertIndex - 1; % Convert 1-start to 0-start
		BegInds(2) = DataInsertIndex    - 1; % Convert 1-start to 0-start
	else
		% In case of a higher level cell array
		NElemsatCurrLevel = length(CellArray);
		for i = 1:NElemsatCurrLevel
			% Insert extry into top PartitionIndex,
			% Flatten corresponding sub Cell Array and update BegInds
			FlatCellArr.PartitionIndex{DepthStartInd}(BegInds(1)+1) = BegInds(2);
			BegInds(2:end) = FlatCellArr.PushFlatCellArrayPartial(CellArray{i}, DepthStartInd+1, BegInds(2:end));
			BegInds(1) = BegInds(1) + 1;
		end
	end
end

