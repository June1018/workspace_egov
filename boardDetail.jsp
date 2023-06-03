<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>   

<%@ taglib prefix="c"      uri="http://java.sun.com/jsp/jstl/core" %>
<%@ taglib prefix="fn"     uri="http://java.sun.com/jsp/jstl/functions" %>

<% pageContext.setAttribute("newline", "\n"); %>
<c:set var="content" value="${fn:replace(boardVO.content, newline, '<br>' ) }"/>

<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>게시판 등록 화면</title>

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

	if( $.trim($("#title").val()) == ""){

		alert("제목을 입력해주세요.");
		$("title").focus();
		return false;
	}

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
			url:"boardWriteSave.do",
			dataType:"text",         // 리턴타입
			success:function(data){  //controller -> ok 
				if(data == "ok"){   
					alert("저장하였습니다.");
					location="boardList.do";
				}else{             //장애 발생
					alert("저장 실패하였습니다. 다시 시도해주세요.");
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
	<table>
		<caption>게시판 상세</caption>
		<tr>
			<th width="20%">제목</th>
			<td width="80%">${boardVO.title }</td>
		</tr>

		<tr>
			<th>글쓴이</th>
			<td><c:out value="${boardVO.name }"/></td>
		</tr>
		<tr>
			<th>내용</th>
			<td height="50"><!-- 6라인 선언해서  대치 \n -> br -->
			${boardVO.content }
			</td>
		</tr>
		<tr>
			<th>등록일</th>
			<td>${boardVO.rdate }</td>
		</tr>
		<tr>
			<th colspan="2">
			<button type="button" onclick="location='boardList.do'">목록</button>
			<button type="button" onclick="location='boardModifyWrite.do?unq=${boardVO.unq}'">수정</button>
			<button type="reset"  onclick="location='passWrite.do?unq=${boardVO.unq }'">삭제</button>
			</th>
			
		</tr>
	</table>
</form>
</body>
</html>