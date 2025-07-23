					eNodeB							|	  |			MME
-----------------------------------------------------------------------------------------------------
AttachRequest(enodeb_id, imsi|m_tmsi ,cgi)			| --> |
													| <-- |		AttachAccept(enodeb_id, mme_id, m_tmsi)
IdentityResponse(enodeb_id, mme_id, imsi, cgi)		| --> |
													|	  |
Paging(m_tmsi, cgi)									| --> |
													|	  |
PathSwitchRequest(enodeb_id, mme_id, cgi)			| --> |
													| <-- |		PathSwitchRequestAcknowledge(enodeb_id, mme_id)
													|	  |
UEContextReleaseCommand(enodeb_id, mme_id, cgi)		| --> |
													| <-- |		UEContextReleaseResponse(enodeb_id, mme_id)
-----------------------------------------------------------------------------------------------------

** call flow 1:
	-> AttachRequest {enodeb_id_1, imsi_1, cgi_1} (регистрация абонента)
	-> IdentityResponse {enodeb_id_1, mme_id_1, imsi_1, cgi_2} (не более чем через 1 сек с AttachRequest) (опциональное сообщение)
	<- AttachAccept {enodeb_id_1, mme_id_1, m_tmsi_1} (не более чем через 1 сек с AttachRequest)

	-> Paging {m_tmsi_1, cgi_3} (приходят регулярно)

	-> PathSwitchRequest {enodeb_id_2, mme_id_1, cgi_4} (новый enodeb_id_2, но старый mme_id_1)
	<- PathSwitchRequestAcknowledge {enodeb_id_2, mme_id_2} (не более чем через 1 сек с PathSwitchRequest, новый mme_id_2)
		*** такие пары PathSwitchRequest и PathSwitchRequestAcknowledge могут приходить регулярно и каждый раз меняется пара (enodeb_id, mme_id)

	-> UEContextReleaseCommand {enodeb_id_2, mme_id_2, cgi_4} (дерегистрация абонента)
	<- UEContextReleaseResponse {enodeb_id_2, mme_id_2} (не более чем через 1 сек с UEContextReleaseCommand)
		*** после этого сбрасываются (enodeb_id, mme_id), но сохраняется m_tmsi_1

	- прошло менее 24 часов
	-> AttachRequest {enodeb_id_3, m_tmsi_1, cgi_5} (нет imsi !!!, но мы запомнили связку (m_tmsi_1, imsi_1) из предыдущих сообщений)
	<- AttachAccept {enodeb_id_3, mme_id_3, m_tmsi_1} (не более чем через 1 сек с AttachRequest)

	-> Paging {m_tmsi_1, cgi_3} (приходят регулярно)

	-> PathSwitchRequest {enodeb_id_4, mme_id_3, cgi_6} (новый enodeb_id_4, но старый mme_id_3)
	<- PathSwitchRequestAcknowledge {enodeb_id_4, mme_id_4} (не более чем через 1 сек с PathSwitchRequest, новый mme_id_4)
		*** такие пары PathSwitchRequest и PathSwitchRequestAcknowledge могут приходить регулярно и каждый раз меняется пара (enodeb_id, mme_id)

	-> UEContextReleaseCommand {enodeb_id_4, mme_id_4, cgi_7} (дерегистрация абонента)
	<- UEContextReleaseResponse {enodeb_id_4, mme_id_4} (не более чем через 1 сек с UEContextReleaseCommand)
		*** после этого сбрасываются (enodeb_id, mme_id), но сохраняется m_tmsi_1

	- прошло более 24 часов (связка (m_tmsi_1, imsi_1) пропала на узле MME) тогда call flow идет с самлого начала:
		--> AttachRequest {enodeb_id_5, imsi_1, cgi_8}
	...

** call flow 2 (небольшая модификация):
	-> AttachRequest {enodeb_id_1, m_tmsi_1, cgi_1} (регистрация абонента)
	-> IdentityResponse {enodeb_id_1, mme_id_1, imsi_1, cgi_2} (не более чем через 1 сек с AttachRequest) (опциональное сообщение)
	<- AttachAccept {enodeb_id_1, mme_id_1, m_tmsi_2} (не более чем через 1 сек с AttachRequest, смена m_tmsi_2 !!!! )
	...
