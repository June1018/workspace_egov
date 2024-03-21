<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>   

<%@ taglib prefix="c"      uri="http://java.sun.com/jsp/jstl/core" %>
<%@ taglib prefix="fn"     uri="http://java.sun.com/jsp/jstl/functions" %>

<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>게시판 수정 화면</title>

<script src="/myproject_new/script/jquery-1.12.4.js"></script>
<script src="/myproject_new/script/jquery-ui.js"></script>

</head>

<style>
board{
	font-size:9pt;
}
button{
	font-size:9pt;
}
table {
	width : 600px;
	border-collapse : collapse; /* 셀 사이의 간격 없애기*/
}
td, th{
	border : 1px solid #cccccc;
	padding : 5px;
}
.input1{
	width:98%;
}
.textarea{
	width:98%;
	height:70px;
}
</style>

<script>
function fn_submit(){
    // trim ()앞뒤공백제거 java, jsp
    // 자바            : document.frm.title.value == ""
    // jquery : $("#title").val() 

    
	if( $("#title").val() == ""){
		alert("제목을 입력해주세요.");
		$("title").focus();
		return false;
	}
    
    $("#title").val($.trim($("#title").val()) );	
    
	if( $.trim($("#pass").val()) == ""){
		alert("암호을 입력해주세요.");
		$("pass").focus();
		return false;
	}
	$("#pass").val($.trim($("#pass").val()) );
	
	var formData = $("#frm").serialize();
	
	//ajax : 비동기 전송방식의 기능을 가지고 있는 jquery의 함수
	$.ajax({
		type:"POST",
		data:formData,
		url:"boardModifySave.do",
		dataType:"text",           // 리턴타입
		/* 전송후 셋팅 */
		success:function(result){  //controller -> ok 
			if(result == "1"){  // ajax : result int리턴값으로 하면 안된다. String 타입으로 해야함 
				alert("저장완료");
				location="boardList.do";
			}else if (result == "-1"){
				alert("암호가 일치하지 않습니다.");
		    }else{                 //장애 발생
				alert("저장 실패\n관리자에게 연락해주세요.");
			}
		},
		error:function(){
			alert("오류발생");
		}		
	});
}

</script>
<body>
<form  id="frm" >
<input type="hidden" name="unq" value="${boardVO.unq }" >

	<table>
		<caption>게시판 수정 화면</caption>
		<tr>
			<th width="20%"><label for="title">제목</label></th>
			<td width="80%"><input type="text" name="title" id="title" class="input1" value="${boardVO.title }"></td>
		</tr>
		<tr>
			<th><label for="pass">암호</label></th>
			<td><input type="password" name="pass" id="pass"></td>
		</tr>
		<tr>
			<th>글쓴이</th>
			<td><input type="text" name="name" id="name" value="${boardVO.name }"/></td>
		</tr>
		<tr>
			<th>내용</th>
			<td><textarea name="content" id="content" class="textarea">${boardVO.content }</textarea>
			</td>
		</tr>
		<tr>
			<th colspan="2">
			<button type="submit" onclick="fn_submit(); return false;">저장</button>
			<button type="reset">취소</button>
			</th>
			
		</tr>
	</table>
</form>
</body>
</html>