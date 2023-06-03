<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
    
<%@ taglib prefix="c"      uri="http://java.sun.com/jsp/jstl/core" %>
<%@ taglib prefix="form"   uri="http://www.springframework.org/tags/form" %>
<%@ taglib prefix="ui"     uri="http://egovframework.gov/ctl/ui"%>
<%@ taglib prefix="spring" uri="http://www.springframework.org/tags"%>

<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>게시판 조회 화면</title>
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
<%@ include file="../include/topmenu.jsp" %>
<br>
<br>
<div class="div1">일반게시판목록</div>
<div class="div2">Total : ${total }</div>
<div class="div2">

<form name="searchFrm" method="post" action="boardList.do">
	<select name="searchGubun" id="searchGubun">
		<option value="title">제목</option>
		<option value="name">글쓴이</option>
		<option value="content">내용</option>
	</select>
	<input type="text" name="searchText" id="searchText">
	<button type="submit">검색</button>
</form>

</div>
<table>
	<tr>
		<th width="15%">번호</th>
		<th width="35%">제목</th>
		<th width="15%">글쓴이</th>
		<th width="20%">등록일</th>
		<th width="15%">조회수</th>
	</tr>
	<c:set var="cnt" value="${rowNumber }" />
	<c:forEach var="result" items="${resultList}">
	
	<tr align="center">
	    <td><c:out value="${cnt }"/></td>
		<td align="left">
		   <a href="boardDetail.do?unq=${result.unq }" ><c:out value="${result.title}"/></a>
		</td>
		<td><c:out value="${result.name }"/></td>
		<td><c:out value="${result.rdate}"/></td>
		<td><c:out value="${result.hits }"/></td>
	</tr>
		<c:set var="cnt" value="${cnt-1 }"/>
	</c:forEach>

</table>
<div style="width:600px;margin-top:5px; text-align:center;">
	<c:forEach var="i" begin="1" end="${totalPage }">
		<a href="boardList.do?viewPage=${i }">${i }</a>
	</c:forEach>
</div>

<div style="width:600px;margin-top:5px; text-align:right;">
	<button type="button" onclick="location='boardWrite.do'">글쓰기</button>
</div>


</body>
</html>