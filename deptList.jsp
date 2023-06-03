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
<title>사원목록</title>
</head>
<style>
	table {
		width : 400px;
		border-collapse : collapse; /* 셀 사이의 간격 없애기*/
	}
	td, th{
		border : 1px solid #cccccc;
		padding : 5px;
	}
</style>

<body>

<table>
    <caption>부서목록</caption>
	<tr>
		<th>부서번호</th>
		<th>부서이름</th>
		<th>부서위치</th>
	</tr>
    <c:forEach var="result" items="${resultList}" varStatus="status">
	<tr>
		<td>${result.deptno}</td>
		<td><a href="deptDetail.do?deptno=${result.deptno }">${result.dname}</a></td>
		<td>${result.loc}</td>
	</tr>
    </c:forEach>
</table>
</body>
</html>