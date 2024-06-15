<!DOCTYPE html>
<html>
<head>
	<meta charset = "UTF-8">
	<meta http-equiv = "refresh" content = "30">
	<style type = "text/css">
		.spec{
			text-align:center;
		}
		.con{
			text-align:left;
		}
		</style>
</head>

<body>
	<h1 align = "center">Aqua Database</h1>
	<div class = "spec">
		# <b>The sensor value description</b>
		<br></br>
	</div>

	<table border = '1' style = "width = 30%" align = "center">
	<tr align = "center">
		<th>ID</th>
		<th>NAME</th>
		<th>ILLU</th>
		<th>TEMP</th>
		<th>DISTANCE</th>
		<th>DATE</th>
	</tr>

	<?php
		$conn = mysqli_connect("localhost", "lmg", "1234");
		mysqli_select_db($conn, "aqua");
		$result = mysqli_query($conn, "select * from sensor");
		while($row = mysqli_fetch_array($result))
		{
			echo "<tr align = center>";
			echo '<th>'.$row['id'].'</td>';
			echo '<th>'.$row['name'].'</td>';
			echo '<th>'.$row['illu'].'</td>';
			echo '<th>',$row['temp'].'</td>';
	      		echo '<th>',$row['distance'].'</td>';
	      		echo '<th>',$row['time'].'</td>';
			echo "</tr>";

		}
		mysqli_close($conn);
	?>
	</table>
</body>
</html>

