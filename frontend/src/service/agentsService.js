import api from "../api/axios";
import endpoints from "../api/endpoints";

const agentService = {
  getAll(params = {}) {
    return api.get(endpoints.agents.list, { params });
  },

  getById(id) {
    return api.get(endpoints.agents.detail(id));
  },

  create(data) {
    return api.post(endpoints.agents.create, data);
  },

  update(id, data) {
    return api.patch(endpoints.agents.detail(id), data);
  },

  delete(id) {
    return api.delete(endpoints.agents.detail(id));
  },

  getLogs(id, params = {}) {
    return api.get(endpoints.agents.logs(id), { params });
  },

  getAlerts(id, params = {}) {
    return api.get(endpoints.agents.alerts(id), { params });
  },
};

export default agentService;
