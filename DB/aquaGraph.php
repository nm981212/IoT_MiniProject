<?php
$conn = mysqli_connect("localhost", "lmg", "1234");
mysqli_select_db($conn, "aqua");
$query = "select name, illu, temp, distance, time from sensor";
$result = mysqli_query($conn, $query);

$data = array(array('time', 'illu', 'temp', 'distance')); 

if($result) {
    while($row = mysqli_fetch_array($result)) {
        array_push($data, array($row['time'], intval($row['illu']), intval($row['temp']), intval($row['distance']))); // Corrected the array elements
    }
}

$options = array(
    'title' => 'Aquarium',
    'width' => 1000, 'height' => 400,
    'curveType' => 'function'
);
?>

<script src="//www.google.com/jsapi"></script>
<script>
var data = <?=json_encode($data) ?>;
var options = <?= json_encode($options) ?>;

google.load('visualization', '1.0', {'packages':['corechart']});

google.setOnLoadCallback(function() {
	var chart = new google.visualization.LineChart(document.querySelector('#chart_div'));
	chart.draw(google.visualization.arrayToDataTable(data), options);
	});
	</script>
<div id="chart_div"></div>

