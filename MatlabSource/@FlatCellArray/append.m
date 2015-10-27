function append(FlatCellArr, Cell2Push, DepthStartInd, InputType, ActualCell2PushDepth)
%APPEND Pushes the Cell2Push into DepthStartInd
%
%    append(FlatCellArr, Cell2Push, DepthStartInd, InputType, ActualCell2PushDepth)
%    
%  INPUT ARGUMENTS
%  
%  * FlatCellArr -
%      
%      This is the Flat Cell Array into which the given Cell2Push is pushed 
%      into.
%      
%  * Cell2Push -
%  
%      This is the cell array to push. It can either be a cell array or a 
%      flattened cell array. The depth of this array must be of depth <=
%      Depth of FlatCellArr. (unless FlatCellArr is empty)
%  
%  * DepthStartInd -
%      
%      This is the depth at which the current Cell2Push is pushed. If the 
%      FlatCellArr is empty (i.e. size(PartitionIndex) = 0), then the depth
%      of FlatCellArr is calculated to be 
%      
%        DepthStartInd + Depth of Cell2Push - 1
%      
%      Note that if DepthStartInd is specified and the array is non-empty, 
%      It is necessary that
%      
%        DepthStartInd + Cell2Push.Depth - 1 = FlatCellArr.Depth
%      
%      If DepthStartInd is unspecified and FlatCellArr is not empty, then 
%      DepthStartInd is calculated according to the above constraint. If it 
%      is empty, then DepthStartInd defaults to 1.
%      
%      In order to force the default calculation of DepthStartInd, pass an 
%      empty vector ([]) to this argument.
%      
%      Note that the term Cell2Push.Depth used above is the final depth 
%      decided taking into account ActualCell2PushDepth
%  
%  * InputType - 
%      
%      In the event that Cell2Push has no vectors, its type cannot be
%      discerned, in such case, it defaults to double. The input InputType,
%      if provided will override the default.
%      
%      In order to force InputType to use default give '' as input.
%      
%      Note that forcing Cell2Push to Flatten out to a particular type is
%      necessary only when pushing into an empty array because in other
%      cases, the Data will always be pushed in the data type of
%      FlatCellArray.
%      
%  * ActualCell2PushDepth - 
%      
%      In the event that the array to be pushed has no vectors, its depth 
%      may be underestimated leading to an error. In such case, the input 
%      provided in ActualCell2PushDepth will take precedence over the 
%      calculated depth. However the constraint listed above must be 
%      satisfied for this new depth too.
%      
%      In the above event, If ActualCell2PushDepth is unspecified and if 
%      DepthStartInd is specified and FlatCellArr.Depth > 0, The function
%      assumes
%      
%        ActualCell2PushDepth = FlatCellArr.Depth - DepthStartInd + 1
%      
%      If DepthStartInd is not given, then ActualCell2PushDepth defaults to 
%      the (possibly underestimated) calculated depth of Cell2Curr.
%      
%      In all cases, ActualCell2PushDepth (when used) must be greater than 
%      the calculated depth for Cell2Push. Also, ActualCell2PushDepth is 
%      ignored if Cell2Push contains any vector.
%      
%  EXAMPLES
%      
%    If B.Depth = 2, C.Depth = 2, then, B.append(C, 1) will append 
%    C.PartitionIndex {1} to B.PartitionIndex{1} and so on. i.e. B will
%    now contain the top level subcells of C.
%    
%    If B.Depth = 2, C.Depth = 1, then, B.append(C, 2) will append 
%    C.PartitionIndex {1} to B.PartitionIndex{2} and so on. i.e. B{end}
%    will now contain, in addition to its own subcells, the subcells of C
%    as well.
%    
%    If B.Depth = 2, C.Depth = 1, then, B.append(C) will be equivalent to
%    B.append(C, 2).
%    
%    If B.Depth = 2, C.Depth = 2, then, B.append(C, 2) will be illegal
%    
%    If B.Depth = 2, C.Depth = 1, then, B.append(C, 1) will be illegal
	
	% Assigning Depth Variables
	CurrFlatArrayDepth = FlatCellArr.Depth;
	if iscell(Cell2Push)
		isDepthUncertain = strcmp(FlatCellArray.getCellType(Cell2Push), 'undecided');
		PushCellArrDepth = FlatCellArray.getCellDepth(Cell2Push); % Calculated depth
		if isDepthUncertain
			% If ActualCell2PushDepth NOT specified
			if nargin() < 5
				% If DepthStartInd is specified and FlatCellArr is non-empty
				if nargin() >=3 && ~isempty(DepthStartInd) && FlatCellArr.Depth
					ActualCell2PushDepth = FlatCellArr.Depth - DepthStartInd + 1;
				else
					ActualCell2PushDepth = PushCellArrDepth;
				end
			% If ActualCell2PushDepth IS specified
			else
				% If ActualCell2PushDepth is less than calculated depth
				if ActualCell2PushDepth < PushCellArrDepth
					ME = MException('FlatCellArray:InvalidInput', 'The Actual Cell Depth cannot be lesser than the Calculated Cell Depth');
					throw(ME);
				end
			end
		else
			ActualCell2PushDepth = PushCellArrDepth;
		end
		PushCellArrDepth = ActualCell2PushDepth;
	elseif isa(Cell2Push, 'FlatCellArray')
		PushCellArrDepth = Cell2Push.Depth;
	elseif isvector(Cell2Push)
		PushCellArrDepth = 0;
	else
		ME = MException('FlatCellArray:InvalidInput', 'The input Cell2Push is neither a Cell Array nor a FlatCellArray nor a vector');
		throw(ME);
	end
	
	% Assigning Default '' value to InputType if not specified
	if nargin() < 4
		InputType = '';
	end
	
	% Assigning Default value (if necessary) to DepthStartInd
	if nargin() < 3
		if CurrFlatArrayDepth == 0
			DepthStartInd = 1;
		else
			DepthStartInd = CurrFlatArrayDepth - PushCellArrDepth + 1;
		end
	end
	
	% Validating DepthStartInd, FlatCellArr.Depth, and Depth of Cell2Push
	if CurrFlatArrayDepth > 0
		if DepthStartInd+PushCellArrDepth-1 ~= CurrFlatArrayDepth
			ME = MException('FlatCellArray:InvalidInput', 'The constraint Cell2Push.Depth+DepthStartInd-1=FlatCellArr.Depth is violated');
			throw(ME);
		end
	end
	
	% Flattening Cell2Push in case it is a cell array
	if iscell(Cell2Push)
		Cell2Push = FlatCellArray.FlattenCellArray(Cell2Push, InputType, PushCellArrDepth);
	end
	
	% Initializing FlatCellArr.PartitionIndex in case FlatCellArr.Depth == 0
	if FlatCellArr.Depth == 0
		CurrFlatArrayDepth = DepthStartInd + PushCellArrDepth - 1;
		FlatCellArr.PartitionIndex = cell(CurrFlatArrayDepth, 1);
		
		FlatCellArr.PartitionIndex(1:DepthStartInd-1) = {zeros(2,1,'uint32')};
		FlatCellArr.PartitionIndex(DepthStartInd:end) = {zeros(1,1,'uint32')};
		
		FlatCellArr.Data = zeros(0,1, class(Cell2Push.Data));
	end
	
	% Storing Insert Indices for all the levels into which appending needs
	% to be done (these indices are 0-start indices)
	BegInds = cellfun(@(x) length(x)-1, FlatCellArr.PartitionIndex(DepthStartInd:end)); % length(all except Beyond the end elem)
	BegInds(end+1) = length(FlatCellArr.Data);
	
	% Reallocating Memory in FlatCellArr and pushing.
	DataExpandLength = length(Cell2Push.Data);
	FlatCellArr.Data(end+1:end+DataExpandLength) = Cell2Push.Data;
	BegInds(end) = BegInds(end) + DataExpandLength;
	
	for i = CurrFlatArrayDepth:-1:DepthStartInd
		% Initializing variables
		LevelExpandLength = length(Cell2Push.PartitionIndex{i+1-DepthStartInd}) - 1; % -1 discounts beyond the end element
		LevelInsertIndex = BegInds(i-DepthStartInd+1) + 1; % Converting to 1-start
		
		% Reallocation and pushing
		FlatCellArr.PartitionIndex{i}(end+1:end+LevelExpandLength) = 0;
		FlatCellArr.PartitionIndex{i}(LevelInsertIndex:LevelInsertIndex+LevelExpandLength-1) = ...
			... 1:end-1 to exclude BTE Elem in Cell2Push + offset by prev Beyond-The-End Elem
			Cell2Push.PartitionIndex{i+1-DepthStartInd}(1:end-1) + FlatCellArr.PartitionIndex{i}(LevelInsertIndex); 
		
		% Assigning Beyond-The-end Element and updating BegInds
		FlatCellArr.PartitionIndex{i}(end) = BegInds(i+1-DepthStartInd+1);
		BegInds(i-DepthStartInd+1) = BegInds(i-DepthStartInd+1) + LevelExpandLength;
	end
	% If the insert level is not the top level, then one must ascertain
	% If there is actually a cell in place to store the appended elements
	% If that is not the case, then we must create a chain of empty cells
	% for that purpose upto one above the given level.
	ActualInsertDepth = 1;
	if length(FlatCellArr.PartitionIndex{1}) == 1
		ActualInsertDepth = 1;
	else
		for i = 1:DepthStartInd-1
			ActualInsertDepth = ActualInsertDepth + 1;
			if FlatCellArr.PartitionIndex{i}(end) == FlatCellArr.PartitionIndex{i}(end-1)
				break;
			end
		end
	end
	
	% if DepthStartInd == CurrFlatArrayDepth + 1 (i.e. appending directly 
	% to data), then the level above DepthStartInd will have to take its
	% value from the size of the Data Array. otherwise, it is taken from
	% the length() - 1 of the lower levels
	if ActualInsertDepth < DepthStartInd
		if DepthStartInd == CurrFlatArrayDepth + 1
			FlatCellArr.PartitionIndex{DepthStartInd-1}(end+1) = length(FlatCellArr.Data);
		end
		for i = DepthStartInd-2:-1:ActualInsertDepth
			FlatCellArr.PartitionIndex{i}(end+1) = length(FlatCellArr.PartitionIndex{i+1}) - 1;
		end
	end
	
	% Reassigning Beyond-The-end Element for one level above ActualInsertDepth
	% (if such a level exists)
	if ActualInsertDepth > 1 && ActualInsertDepth <= CurrFlatArrayDepth
		FlatCellArr.PartitionIndex{ActualInsertDepth-1}(end) = length(FlatCellArr.PartitionIndex{ActualInsertDepth}) - 1;
	elseif ActualInsertDepth > 1 && ActualInsertDepth == CurrFlatArrayDepth + 1
		FlatCellArr.PartitionIndex{ActualInsertDepth-1}(end) = length(FlatCellArr.Data);
	end
end

