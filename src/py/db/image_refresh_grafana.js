
//Images must be in /usr/share/grafana/public/img/

<img src="/public/img/my_image.jpg" id="myImage">

<script language="text/javascript">
var myImageElement = document.getElementById('myImage');
myImageElement.src = '/public/img/my_image.jpg?rand=' + Math.random();

setInterval(function() {
    var myImageElement = document.getElementById('myImage');
    myImageElement.src = '/public/img/my_image.jpg?rand=' + Math.random();
}, 10000);
</script> 
