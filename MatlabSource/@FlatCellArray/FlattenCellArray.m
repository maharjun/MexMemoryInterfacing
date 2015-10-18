function obj = FlattenCellArray(CellArray, InputArrayType, ActualDepth)
% FlattenCellArray - Returns flattened version of Input Cell Array
% 
%     obj = FlattenCellArray(CellArray, InputArrayType, ActualDepth)
% 
%   This function returns the flattened cell array version of
%   the given Input Cell Array. It returns an error if the cell
%   array is not a valid cell array. The depth of the array is
%   automatically calculated using CellArrayDepth.

	obj = FlatCellArray();
	
	% Setting Array Type and validating cell array
	ArrayTypeCalc = FlatCellArray.getCellType(CellArray);
	ArrayType = ArrayTypeCalc;
	
	if strcmp(ArrayTypeCalc, 'error')
		Exception = MException('FlatCellArray:InvalidInput', 'The input array given is either not a cell array or not a cell array of uniform type');
		throw(Exception);
	elseif strcmp(ArrayTypeCalc, 'undecided')
		if nargin < 2 || (nargin >= 2 && strcmp(InputArrayType,''))
			warning('FlatCellArray:UndecidedType', ...
					['The cell array does not seem to contain any vectors and thus,' ...
					 ' the type is taken to be double by default. Specify type as' ...
					 ' additional argument if otherwise required']);
		end
		if nargin < 3
			warning('FlatCellArray:UndecidedDepth', ...
					['The cell array does not seem to contain any vectors and thus,' ...
					 ' the Depth of the cell array may possibly be incorrectly judged.' ...
					 ' Specify Actual Depth as additional argument if otherwise required']);
		end
		
		ArrayType = 'double';
	end
	if nargin >= 2 && ~strcmp(InputArrayType,'')
		ArrayType = InputArrayType;
	end
	
	% Getting Caculated Cell Array Depth and Sizes
	CellArrayDepth = FlatCellArray.getCellDepth(CellArray);
	CellLevelSizes = FlatCellArray.getCellLevelSizes(CellArray);
	if nargin() < 3
		ActualDepth = CellArrayDepth;
	end
	
	% Validating ActualDepth
	if ActualDepth < CellArrayDepth
		ME = MException('FlatCellArray:InvalidInput', 'The Actual Cell Depth cannot be lesser than the Calculated Cell Depth');
		throw(ME);
	end
	if ~strcmp(ArrayTypeCalc, 'undecided') && nargin() == 3 && ActualDepth ~= CellArrayDepth
		warning('FlatCellArray:RedundantDepthInput', 'The Depth for the cell array is NOT undecided. Ignoring given ActualDepth');
		ActualDepth = CellArrayDepth;
	end
	
	% Initializing PartitionIndex
	obj.PartitionIndex = cell(ActualDepth, 1);
	for i = 1:CellArrayDepth
		obj.PartitionIndex{i} = zeros(CellLevelSizes(i)+1, 1, 'uint32');
	end
	for i = CellArrayDepth+1:ActualDepth
		obj.PartitionIndex{i} = zeros(1,1,'uint32');
	end
	
	% Initializing Data
	obj.Data = zeros(CellLevelSizes(CellArrayDepth + 1), 1, ArrayType);
	
	%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	% Recursive Flattening %%%%%
	%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	BegInds = obj.PushFlatCellArrayPartial(CellArray, 1, zeros(CellArrayDepth+1, 1));
	
	% Inserting Beyond-The-End elements
	for i = 1:CellArrayDepth
		obj.PartitionIndex{i}(end) = BegInds(i+1);
	end
end

