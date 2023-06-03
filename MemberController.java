package main.web;

import java.util.List;
import javax.annotation.Resource;
import javax.servlet.http.HttpSession;
import org.springframework.stereotype.Controller;
import org.springframework.ui.ModelMap;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.ResponseBody;
import main.service.MemberService;
import main.service.MemberVO;

@Controller
public class MemberController {
	
	@Resource(name="memberService")  //서비스 리소스 선언을 해야 사용할 수 있음.
	private MemberService memberService;
	
	/*
	@RequestMapping("memberWrite.do")
	public String memberWrite() {
		return "member/memberWrite";
	}
    */

	/*
	 * 회원등록처리
	 */
	@RequestMapping("/memberWriteSave.do")
	@ResponseBody
	public String insertMember(MemberVO vo) throws Exception{
		String message = "";

		String result = memberService.insertMember(vo);
	System.out.println("result :"+result);
		if (result == null) {  //null일때 성공
			message = "ok";
		}
		return message;
	}

	@RequestMapping("/idcheck.do")
	@ResponseBody
	public String selectMemberIdCheck(String userid) throws Exception {
		String message = "";

		int count = memberService.selectMemberIdCheck(userid);
		if (count == 0) {  //null일때 성공
			message = "ok";
		}
		return message;
	}
	@RequestMapping("/post1.do")
	public String post1() {
		return "member/post1";
	}
	
	@RequestMapping("/post2.do")
	public String post2(String dong, ModelMap model) throws Exception{
		List<?> list = memberService.selectPostList(dong);
		model.addAttribute("resultList", list);
		return "member/post2";
	}
	
	@RequestMapping("/loginWrite.do")
	public String loginWrite() {
		return "member/loginWrite";
	}
	
	@RequestMapping("/loginWriteSub.do")
	@ResponseBody	
	public String loginProcessing(MemberVO vo, HttpSession session) throws Exception {
		String message = "";
		int count = memberService.selectMemberCount(vo);

		if (count == 1) {  //null일때 성공
			// session 생성
			session.setAttribute("SessionUserID", vo.getUserid());   //사용자가 입력ID = getUserid
			// message 처리
			message = "ok";
		}
		return message;
	}
	
	@RequestMapping("/logout.do")
	public String loginOut(HttpSession session) {
		System.out.println("logout"+session);
		session.removeAttribute("SessionUserID");   //log out 처리
		return "member/loginWrite";
	}
	
}
