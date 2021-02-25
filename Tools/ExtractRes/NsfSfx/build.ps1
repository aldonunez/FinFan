param ( [string] $extDir = ".\ext" )

$dataDir = "$PSScriptRoot\..\Data"
$objDir = "$PSScriptRoot\obj"
$baseName = "nsf-sfx"
$mapPath = "$objDir\$baseName.map"
$specPath = "$dataDir\$baseName.csv"

get-command $extDir\ca65, $extDir\ld65 > $null 2>&1
if ( !$? )
{
	throw "Missing external in $extDir`: ca65, ld65"
}

mkdir $objDir -ErrorAction ignore > $null

& $extDir\ca65 $PSScriptRoot\$baseName.asm -o $objDir\$baseName.o
& $extDir\ld65 -C $PSScriptRoot\$baseName.cfg $objDir\$baseName.o -o $dataDir\$baseName.bin -m $mapPath

$inSegmentList = $false
$segmentHeaderLineNumber
$lineNumber = 0
$segments = @{}

foreach ( $line in gc $mapPath )
{
	$lineNumber++

	if ( $line.StartsWith( "Segment list:" ) )
	{
		$inSegmentList = $true
		$segmentHeaderLineNumber = $lineNumber
	}
	elseif ( $inSegmentList -and $lineNumber -ge $segmentHeaderLineNumber + 4 )
	{
		if ( $line -notmatch "^([_0-9A-Z]+) +([0-9A-F]+) +[0-9A-F]+ +([0-9A-F]+) +[0-9A-F]+" )
		{
			break
		}
		else
		{
			$entry = @{
				Name = $matches[1];
				Address = [Convert]::ToInt32( $matches[2], 16 );
				Size = [Convert]::ToInt32( $matches[3], 16 )
			}
			$segments[$entry.Name] = $entry
		}
	}
}

# The output NSF is arranged as follows (bank refers to 4 KiB NSF banks, not NES mapper banks):
# - header: $80 bytes
# - bank 0: custom code, padded to full bank size
# - bank 1: empty (filled with zero)
# - bank 2: code beginning at $A000
# - bank 3: code beginning at $B000
# - bank 4: code beginning at $C000
# - bank 5: code beginning at $D000
# - bank 6: code beginning at $E000
# - bank 7: code beginning at $F000
#
# The Bankswitch Init values in the header are all zero to reflect this layout.

$customBeforeExitSize = $segments["EXIT"].Address - 0x8000
$customBeforeExitSizeHex = "{0:X}" -f $customBeforeExitSize
$exitSizeHex = "{0:X}" -f $segments["EXIT"].Size
$exitSrcOffsetHex = "{0:X}" -f ($segments["EXIT"].Address - 0x8000 + $segments["HEADER"].Size)
$nsfSizeHex = "{0:X}" -f 0x8080

function GetNsfOffset( $romAddr )
{
	$romAddr - 0x8000 + 0x80
}

function redirect( $path, [parameter(ValueFromPipeline)] $p ) { $p | out-file -encoding ascii $path }
function append( $path, [parameter(ValueFromPipeline)] $p ) { $p | out-file -encoding ascii -append $path }

echo "src_name,    src_addr, dst_addr, length" | redirect $specPath
echo ",            0,        0,        $nsfSizeHex" | append $specPath
echo ("rom,         2E04D,    {0:X},     15" -f (GetNsfOffset 0xA03D)) | append $specPath		# Chaos rumble
echo ("rom,         3AD94,    {0:X},     2F" -f (GetNsfOffset 0xAD84)) | append $specPath		# Menu confirm and cursor
echo ("rom,         36EDF,    {0:X},     75" -f (GetNsfOffset 0xAECF)) | append $specPath		# Minigame error
echo ("rom,         33EC8,    {0:X},     148" -f (GetNsfOffset 0xBEB8)) | append $specPath		# Battle SFX (magic, hurt, strike)
echo ("rom,         3C010,    {0:X},     4000" -f (GetNsfOffset 0xC000)) | append $specPath
echo ("$baseName.bin, 0,        0,        80") | append $specPath								# Header
echo ("$baseName.bin, 80,       80,       $customBeforeExitSizeHex") | append $specPath			# Custom
echo ("$baseName.bin, $exitSrcOffsetHex,      {0:X},     $exitSizeHex" -f (GetNsfOffset 0xFEA8)) | append $specPath

# The last entry overwrites the procedure that waits for a VBLANK, with a custom
# implementation that forcibly returns.
