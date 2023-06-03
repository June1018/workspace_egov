<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>

<%@ taglib prefix="c"      uri="http://java.sun.com/jsp/jstl/core" %>

<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>암호입력화면</title>
<script src="/myproject_new/script/jquery-1.12.4.js"></script>
<script src="/myproject_new/script/jquery-ui.js"></script>
</head>

<script>
$(function(){

		$("#delBtn").click(function(){
			
			//삭제 패스워드 체크
			var pass = $("#pass").val();
			pass = $.trim(pass);
			if( pass == ""){
				alert("암호을 입력해주세요.");
				$("#pass").focus();
				return false;
		    }


			var sendData = "unq=${unq}&pass="+$("#pass").val();
	
			//ajax : 비동기 전송방식의 기능을 가지고 있는 jquery의 함수
			$.ajax({
				type:"POST",
				data:sendData,             // json설정
				url:"boardDelete.do",
				dataType:"text",           // 리턴타입
				
				/* 전송후 셋팅 */
				success:function(result){  //controller -> (1) ok 
					alert("result"+result);
					if(result == "1"){     // ajax : result int리턴값으로 하면 안된다. String 타입으로 해야함 
						alert("삭제완료");
						location="boardList.do";
					}else if (result == "-1"){
						alert("암호가 일치하지 않습니다.");
				    }else{                 
						alert("삭제 실패\n관리자에게 연락해주세요.");
					}
				},
				error:function(){          //장애 발생
					alert("오류발생");
				}		
		});
	});
});

</script>

<body>
<table>
	<tr>
		<th>암호입력</th>
		<td><input type="password" id="pass"></td>
		<td><button type="button" id="delBtn">삭제하기</button></td>
	</tr>
</table>
</body>
</html>