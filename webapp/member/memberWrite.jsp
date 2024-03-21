<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
    
<!DOCTYPE html>
<html>
<head>
<style>
body{
	font-size:9pt;
	font-color:#333333;
	font-family:맑은고딕;
}

a{
	text-decoration:none;
}
table{
	width:600px;
	border-collapse:collapse;
}
th, td{
	border:1px solid #cccccc;
	padding:3px;
	line-height:2;  
}
caption{
    font-size:15pt;
    font-weight:bold;
    margin-top:10px;
    padding-bottom:5px;
}
.div_button{
	width:600px;
	text-align:center;
	margin-top:5px;
}
<!-- line-height:2 주소값 사이 간격(2)-->
</style>
<meta charset="UTF-8">
<title>회원가입화면</title>

<link rel="stylesheet" href="//code.jquery.com/ui/1.13.2/themes/base/jquery-ui.css">
<link rel="stylesheet" href="/resources/demos/style.css">
<script src="/myproject_new/script/jquery-1.12.4.js"></script>
<script src="/myproject_new/script/jquery-ui.js"></script>

 <script>
 $( function() {
   $( "#birth" ).datepicker({
	   changeMonth: true,
	   changeYear: true
   });
   $("#btn_zipcode").click(function (){
	   var w = 500;
	   var h = 100;
	   var url = "post1.do";
	   window.open(url, 'zipcode','width='+w,'height='+h);
   });


   $("#btn_idcheck").click(function(){ 
	   var userid = $.trim($("#userid").val());  //앞뒤공백 제거
	   if( userid == ""){
	       alert("아이디를 입력해주세요.");
		   $("#userid").focus();
		   return false;
       }
	   
	   
	   //idcheck.do로 데이터 전송
	   //ajax : 비동기 전송방식의 기능을 가지고 있는 jquery의 함수
	   $.ajax({
		   type:"POST",
		   data:"userid="+userid,     // json 타입으로 전송 
		   url:"idcheck.do",
		   dataType:"text",           // 리턴타입
		   /* 전송후 셋팅 */
		   success:function(result){  //controller -> ok 
		       if(result == "ok"){    // ajax : result int리턴값으로 하면 안된다. String 타입으로 해야함 
				   alert("사용가능한 아이디 입니다.");
 			   }else{                 
				   alert("이미사용중인 아이디입니다.");
			   }
			},
			error:function(){
		       alert("오류발생");
			}		
		});
   });
   

   
   $("#btn_submit").click(function(){
	   var userid = $("#userid").val();
	   var pass  = $("#pass").val();
	   var name  = $("#name").val();
	   userid = $.trim(userid);
	   pass   = $.trim(pass);
	   name   = $.trim(name);

	   if( userid == ""){
	       alert("아이디를 입력해주세요.");
		   $("#userid").focus();
		   return false;
       }
	   if( pass == ""){
	       alert("암호을 입력해주세요.");
	       $("#pass").focus();
		   return false;
       }
	   if( name == ""){
	       alert("이름을 입력해주세요.");
		   $("#name").focus();
		   return false;
	   }
	   
	   $("#userid").val(userid);
	   $("#pass").val(pass);
	   $("#name").val(name);
	      
	   var formData = $("#frm").serialize();  /* form 아래의 데이터   */
	   alert("formData"+$("#frm").serialize());
	   //ajax : 비동기 전송방식의 기능을 가지고 있는 jquery의 함수
	   $.ajax({
		   type:"POST",
		   data:formData,
		   url:"memberWriteSave.do",
		   dataType:"text",           // 리턴타입
		   /* 전송후 셋팅 */
		   success:function(result){  //controller -> ok 
		       if(result == "ok"){  // ajax : result int리턴값으로 하면 안된다. String 타입으로 해야함 
				   alert("저장완료");
				   location="loginWrite.do";
			   }else{                 //장애 발생
				   alert("저장 실패");
			   }
			},
			error:function(){
		       alert("오류발생");
			}		
		});	   
   });   
   
 } );
 </script>
</head>
<body>

<%@ include file="../include/topmenu.jsp" %>

<form name="frm" id="frm">
<table>
    <caption>회원가입 폼</caption>
    <tr>
    	<th><label for="userid">아이디</label></th>
    	<td>
    	<input type="text" name="userid" id="userid" placeholder="아이디입력">  <!-- 초기출력시 아이디입력 나옴 -->
    	<button type="button" id="btn_idcheck">중복체크</button>
    	</td>
    </tr>
    <tr>
    	<th><label for="pass">암호</label></th>
    	<td>
    	<input type="password" name="pass" id="pass">
    	</td>
    </tr>
    <tr>
    	<th><label for="name">이름</label></th>
    	<td>
    	<input type="text" name="name" id="name">
    	</td>
    </tr>
    <tr>
    	<th><label for="gender">성별</label></th>
    	<td>
    	<input type="radio" name="gender" id="gender" value="M">남
    	<input type="radio" name="gender" id="gender" value="F">여
    	</td>
    </tr>
    <tr>
    	<th><label for="birth">생년월일</label></th>
    	<td>
    	<input type="text" name="birth" id="birth">
    	</td>
    </tr>
    <tr>
    	<th><label for="phone">연락처</label></th>
    	<td>
    	<input type="text" name="phone" id="phone">(예:010-1234-1234)
    	</td>
    </tr>    
    <tr>
    	<th><label for="address">주소</label></th>
    	<td>
	    	<input type="text" name="zipcode" id="zipcode">
	    	<button type="button" id="btn_zipcode">우편번호찾기</button>
	    	<br>
	    	<input type="text" name="address" id="address">
    	</td>
    </tr>
</table>
<div class="div_button">
 	<button type="button" id="btn_submit">저장</button>
 	<button type="reset" id="btn_reset">취소</button>
</div>
</form>
 
</body>
</html>