<?php
class BaseNComponent extends Object {
	/*!
	* @brief Parses a base-N object out of a string and a reverse character set
	*
	* @param $str Input string
	* @param $rcharset Reverse character set
	*
	* @return Base-N index array
	*/
	function make($str, $rcharset)
	{
		$ret = array();
		for($i=0; $i<strlen($str); $i++)
			$ret[$i] = $rcharset[$str[$i]];
		return $ret;
	}

	/*!
	* @brief Adds an integer to a base-N object (using ripple carry)
	*
	* @param $num Input array
	* @param $base Base of $num
	* @param $add Value to add to $num

	* @return Base-N index array
	*/
	function add($num, $base, $add)
	{
		$len = count($num);

		//Get number of digits required to add this number
		$maxplace = ceil( log($add) / log($base) );

		//Too much? Can't possibly fit, saturate and quit
		if($maxplace > $len)
		{
			for($i=0; $i<$len; $i++)
				$num[$i] = $base-1;
			return $num;
		}

		//No, we will fit. Bump the rightmost digit
		$num[$len-1] += $add;

		//Handle carries
		$bOverflow = false;
		$pvalue = 1;
		for($i=$len-1; $i>=0; $i--)
		{
			//Less than base? We're done
			if($num[$i] < $base)
				break;

			//Greater than base? Handle carry
			$low = $num[$i] % $base;
			$high = floor($num[$i] / $base);
			$num[$i] = $low;

			//Carry off end? Overflow
			if($high != 0 && $i==0)
			{
				$bOverflow = true;
				break;
			}

			$num[$i-1] += $high;
		}

		//Saturate in case of overflow
		if($bOverflow)
		{
			for($i=0; $i<$len; $i++)
				$num[$i] = $base-1;
		}

		//Done
		return $num;
	}

	/*!
	* @brief Converts a base-N integer to a string
	*
	* @param $num Input array
	* @param $charset Character set
	*
	* @return Output string
	*/
	function toString($num, $charset)
	{
		$str = '';
		for($i=0; $i<count($num); $i++)
			$str .= $charset[$num[$i]];
		return $str;
	}

	/*!
	* @brief Converts a base-N integer to a float
	*
	* @param $num Input array
	* @param $charset Character set
	*
	* @return Output value
	*/
	function toFloat($num, $charset)
	{
		$ret = 0;
		$len = count($num);
		$base = strlen($charset);

		for($i=0; $i<$len; $i++)
		{
			$pval = pow($base, $len-1-$i);
			$ret += ($num[$i]+1) * $pval;
		}

		return $ret;
	}

}
?>