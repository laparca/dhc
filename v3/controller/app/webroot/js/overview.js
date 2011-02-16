/******************************************************************************
*                                                                             *
* Distributed Hash Cracker v3.0                                               *
*                                                                             *
* Copyright (c) 2009 RPISEC.                                                  *
* Copyright (C) 2010 Samuel Rodríguez Sevilla                                 *
* All rights reserved.                                                        *
*                                                                             *
* Redistribution and use in source and binary forms, with or without modifi-  *
* cation, are permitted provided that the following conditions are met:       *
*                                                                             *
*    * Redistributions of source code must retain the above copyright notice  *
*      this list of conditions and the following disclaimer.                  *
*                                                                             *
*    * Redistributions in binary form must reproduce the above copyright      *
*      notice, this list of conditions and the following disclaimer in the    *
*      documentation and/or other materials provided with the distribution.   *
*                                                                             *
*    * Neither the name of RPISEC nor the names of its contributors may be    *
*      used to endorse or promote products derived from this software without *
*      specific prior written permission.                                     *
*                                                                             *
* THIS SOFTWARE IS PROVIDED BY RPISEC "AS IS" AND ANY EXPRESS OR IMPLIED      *
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF        *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN     *
* NO EVENT SHALL RPISEC BE HELD LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED    *
* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR      *
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      *
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING        *
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS          *
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                *
*                                                                             *
******************************************************************************/

/* It retrieve the values each 10 seconds */
var interval = 10;

/**
 * Requiere variable history_url from main template
 */
function loadChart() {
	$.ajax({
		url: history_url,
		dataType: 'json',
		success: function(data) {
			var values = [];
			values[0] = [];
			for(i = 0; i < data.length; i++) {
				var date = new Date(data[i].History.time);
				values[0][i] = [date.toUTCString(), parseFloat(data[i].History.speed)];
			}
			
			if(data.length > 1) {
				$('#OverviewChart').html('');
				$.jqplot('OverviewChart', values, {
					seriesDefaults: {
						showMarker:false
					},
					axes: {
						xaxis: {
							renderer: $.jqplot.DateAxisRenderer,
							tickOptions: {
								formatString: '%Y-%m-%d %H:%M'
							}
						}
					}
				});
			}
		},
		error: function(jqXHR, textStatus, errorThrown) {
			alert('ERROR: '+textStatus);
		}
	});	
	setTimeout(loadChart, interval * 1000);
}

$(function() {
	//setTimeout(loadChart, interval * 1000);
	loadChart();
});
