function setHeading(name) {
  let myHeading = document.querySelector('h2');
  myHeading.textContent = '用户名:' + name;
}

function setUserName() {
  let myName = prompt('请输入你的名字');
  localStorage.setItem('name', myName);
  setHeading(myName);
} 

let storedName = localStorage.getItem('name');

if(!storedName) {

   setUserName();
} 
else {
	
   setHeading(storedName);
}

let myButton = document.querySelector('button'); 
myButton.onclick = setUserName;
