//--------------------------------------------------
function drawLine(ctx, startX, startY, endX, endY){
	ctx.beginPath();
	ctx.moveTo(startX,startY);
	ctx.lineTo(endX,endY);
	ctx.stroke();
}

//--------------------------------------------------
function drawArc(ctx, centerX, centerY, radius, startAngle, endAngle){
	ctx.beginPath();
	ctx.arc(centerX, centerY, radius, startAngle, endAngle);
	ctx.stroke();
}

//--------------------------------------------------
function drawPieSlice(ctx,centerX, centerY, radius, startAngle, endAngle, color ){
	ctx.fillStyle = color;
	ctx.beginPath();
	ctx.moveTo(centerX,centerY);
	ctx.arc(centerX, centerY, radius, startAngle, endAngle);
	ctx.closePath();
	ctx.fill();
}

//--------------------------------------------------
var Piechart = function(options){
	this.options = options;
	this.canvas = options.canvas;
	this.ctx = this.canvas.getContext("2d");
	this.colors = options.colors;

	this.draw = function(){
		var total_value = 0;
		var color_index = 0;
		for (var categ in this.options.data){
			var val = this.options.data[categ];
			total_value += val;
		}

		var start_angle = 0;
		for (categ in this.options.data){
			val = this.options.data[categ];
			var slice_angle = 2 * Math.PI * val / total_value;

			drawPieSlice(
				this.ctx,
				this.canvas.width/2,
				this.canvas.height/2,
				Math.min(this.canvas.width/2,this.canvas.height/2),
				start_angle,
				start_angle+slice_angle,
				this.colors[color_index%this.colors.length]
				);

			start_angle += slice_angle;
			color_index++;
		}

		//drawing a white circle over the chart to create the doughnut chart
		if (this.options.doughnutHoleSize){
			drawPieSlice(
				this.ctx,
				this.canvas.width/2,
				this.canvas.height/2,
				this.options.doughnutHoleSize * Math.min(this.canvas.width/2,this.canvas.height/2),
				0,
				2 * Math.PI,
				"#ff0000"
				);
		}

		//drawing the pourcent of each slice
		start_angle = 0;
		for (categ in this.options.data){
			val = this.options.data[categ];
			slice_angle = 2 * Math.PI * val / total_value;
			var pieRadius = Math.min(this.canvas.width/2,this.canvas.height/2);
			var labelX = this.canvas.width/2 + (pieRadius / 2) * Math.cos(start_angle + slice_angle/2);
			var labelY = this.canvas.height/2 + (pieRadius / 2) * Math.sin(start_angle + slice_angle/2);

			if (this.options.doughnutHoleSize){
				var offset = (pieRadius * this.options.doughnutHoleSize ) / 2;
				labelX = this.canvas.width/2 + (offset + pieRadius / 2) * Math.cos(start_angle + slice_angle/2);
				labelY = this.canvas.height/2 + (offset + pieRadius / 2) * Math.sin(start_angle + slice_angle/2);               
			}

			var labelText = Math.round(100 * val / total_value);
			this.ctx.fillStyle = "white";
			this.ctx.font = "bold 20px Arial";
			this.ctx.fillText(labelText+"%", labelX,labelY);
			start_angle += slice_angle;
		}

		//drawing the legend
		if (this.options.legend){
			color_index = 0;
			var legendHTML = "";
			for (categ in this.options.data){
				legendHTML += "<div><span style='display:inline-block;width:20px;background-color:"+this.colors[color_index++]+";'>&nbsp;</span> "+categ+"</div>";
			}
			this.options.legend.innerHTML = legendHTML;
		}
	}
}